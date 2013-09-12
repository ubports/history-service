/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "historydaemon.h"
#include "telepathyhelper.h"
#include "itemfactory.h"
#include "thread.h"
#include "manager.h"
#include "textevent.h"
#include "texteventattachment.h"
#include "voiceevent.h"

#include "pluginmanager.h"
#include "plugin.h"
#include "reader.h"
#include "writer.h"

#include <QStandardPaths>
#include <QCryptographicHash>
#include <TelepathyQt/CallChannel>

HistoryDaemon::HistoryDaemon(QObject *parent)
    : QObject(parent), mCallObserver(this), mTextObserver(this)
{
    // FIXME: maybe we should look for both at once
    // try to find a plugin that has a reader
    Q_FOREACH(History::PluginPtr plugin, History::PluginManager::instance()->plugins()) {
        mReader = plugin->reader();
        if (mReader) {
            break;
        }
    }

    // and now a plugin that has a writer
    Q_FOREACH(History::PluginPtr plugin, History::PluginManager::instance()->plugins()) {
        mWriter = plugin->writer();
        if (mWriter) {
            break;
        }
    }

    connect(TelepathyHelper::instance(),
            SIGNAL(channelObserverCreated(ChannelObserver*)),
            SLOT(onObserverCreated()));

    connect(&mCallObserver,
            SIGNAL(callEnded(Tp::CallChannelPtr)),
            SLOT(onCallEnded(Tp::CallChannelPtr)));
    connect(&mTextObserver,
            SIGNAL(messageReceived(Tp::TextChannelPtr,Tp::ReceivedMessage)),
            SLOT(onMessageReceived(Tp::TextChannelPtr,Tp::ReceivedMessage)));
    connect(&mTextObserver,
            SIGNAL(messageSent(Tp::TextChannelPtr,Tp::Message,QString)),
            SLOT(onMessageSent(Tp::TextChannelPtr,Tp::Message,QString)));
    connect(&mTextObserver,
            SIGNAL(messageRead(Tp::TextChannelPtr,Tp::ReceivedMessage)),
            SLOT(onMessageRead(Tp::TextChannelPtr,Tp::ReceivedMessage)));

    // FIXME: we need to do this in a better way, but for now this should do
    mProtocolFlags["ofono"] = History::MatchPhoneNumber;

    mDBus.connectToBus();
}

HistoryDaemon::~HistoryDaemon()
{
}

HistoryDaemon *HistoryDaemon::instance()
{
    static HistoryDaemon *self = new HistoryDaemon();
    return self;
}

History::ThreadPtr HistoryDaemon::threadForParticipants(const QString &accountId,
                                                        History::EventType type,
                                                        const QStringList &participants,
                                                        History::MatchFlags matchFlags,
                                                        bool create)
{
    History::ThreadPtr thread = mReader->threadForParticipants(accountId,
                                                               type,
                                                               participants,
                                                               matchFlags);
    if (thread.isNull() && create) {
        thread = mWriter->createThreadForParticipants(accountId, type, participants);
    }
    return thread;
}

void HistoryDaemon::onObserverCreated()
{
    qDebug() << __PRETTY_FUNCTION__;
    ChannelObserver *observer = TelepathyHelper::instance()->channelObserver();

    connect(observer, SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
            &mCallObserver, SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));
    connect(observer, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
            &mTextObserver, SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));
}

void HistoryDaemon::onCallEnded(const Tp::CallChannelPtr &channel)
{
    qDebug() << __PRETTY_FUNCTION__;
    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, channel->remoteMembers()) {
        participants << contact->id();
    }

    QString accountId = channel->property(History::FieldAccountId).toString();
    History::ThreadPtr thread = threadForParticipants(accountId,
                                                      History::EventTypeVoice,
                                                      participants,
                                                      matchFlagsForChannel(channel),
                                                      true);
    // fill the call info
    QDateTime timestamp = channel->property(History::FieldTimestamp).toDateTime();

    // FIXME: check if checking for isRequested() is enough
    bool incoming = !channel->isRequested();
    QTime duration(0, 0, 0);
    bool missed = incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer;

    if (!missed) {
        QDateTime activeTime = channel->property("activeTimestamp").toDateTime();
        duration = duration.addSecs(activeTime.secsTo(QDateTime::currentDateTime()));
    }

    QString eventId = QString("%1:%2").arg(thread->threadId()).arg(timestamp.toString());
    History::VoiceEventPtr event = History::ItemFactory::instance()->createVoiceEvent(thread->accountId(),
                                                                                      thread->threadId(),
                                                                                      eventId,
                                                                                      incoming ? channel->initiatorContact()->id() : "self",
                                                                                      timestamp,
                                                                                      missed, // only mark as a new (unseen) event if it is a missed call
                                                                                      missed,
                                                                                      duration);
    mWriter->writeVoiceEvent(event);
}

void HistoryDaemon::onMessageReceived(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message)
{
    qDebug() << __PRETTY_FUNCTION__;

    // ignore delivery reports for now.
    // FIXME: maybe we should set the readTimestamp when a delivery report is received
    if (message.isDeliveryReport() || message.isRescued() || message.isScrollback()) {
        return;
    }

    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupContacts(false)) {
        participants << contact->id();
    }

    History::ThreadPtr thread = threadForParticipants(textChannel->property("accountId").toString(),
                                                                            History::EventTypeText,
                                                                            participants,
                                                                            matchFlagsForChannel(textChannel),
                                                                            true);
    int count = 1;
    History::TextEventPtr event;
    History::TextEventAttachments attachments;
    History::MessageType type = History::MessageTypeText;
    QString subject;

    if (message.hasNonTextContent()) {
        QString normalizedAccountId = QString(QCryptographicHash::hash(thread->accountId().toLatin1(), QCryptographicHash::Md5).toHex());
        QString normalizedThreadId = QString(QCryptographicHash::hash(thread->threadId().toLatin1(), QCryptographicHash::Md5).toHex());
        QString normalizedEventId = QString(QCryptographicHash::hash(message.messageToken().toLatin1(), QCryptographicHash::Md5).toHex());
        QString mmsStoragePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

        type = History::MessageTypeMultiParty;
        subject = message.header()["subject"].variant().toString();

        QDir dir(mmsStoragePath);
        if (!dir.exists("history-service") && !dir.mkpath("history-service")) {
            qDebug() << "Failed to create dir";
            return;
        }
        dir.cd("history-service");

        Q_FOREACH(const Tp::MessagePart &part, message.parts()) {
            // ignore the header part
            if (part["content-type"].variant().toString().isEmpty()) {
                continue;
            }
            mmsStoragePath = dir.absoluteFilePath(QString("attachments/%1/%2/%3/").
                                                  arg(normalizedAccountId,
                                                      normalizedThreadId,
                                                      normalizedEventId));

            QFile file(mmsStoragePath+QString::number(count++));
            if (!dir.mkpath(mmsStoragePath) || !file.open(QIODevice::WriteOnly)) {
                qWarning() << "Failed to save attachment";
                continue;
            }
            file.write(part["content"].variant().toByteArray());
            file.close();
            History::TextEventAttachmentPtr attachment = History::TextEventAttachmentPtr(new History::TextEventAttachment(
                                                         thread->accountId(),
                                                         thread->threadId(),
                                                         message.messageToken(),
                                                         part["identifier"].variant().toString(),
                                                         part["content-type"].variant().toString(),
                                                         file.fileName()));
            attachments << attachment;
        }
    }

    event = History::ItemFactory::instance()->createTextEvent(thread->accountId(),
                                                              thread->threadId(),
                                                              message.messageToken(),
                                                              message.sender()->id(),
                                                              message.received(),
                                                              true, // message is always unread until it reaches HistoryDaemon::onMessageRead
                                                              message.text(),
                                                              type,
                                                              History::MessageFlags(),
                                                              QDateTime(),
                                                              subject,
                                                              attachments);

    mWriter->writeTextEvent(event);
}

void HistoryDaemon::onMessageRead(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message)
{
    qDebug() << __PRETTY_FUNCTION__;
    // FIXME: implement
}

void HistoryDaemon::onMessageSent(const Tp::TextChannelPtr textChannel, const Tp::Message &message, const QString &messageToken)
{
    qDebug() << __PRETTY_FUNCTION__;
    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupContacts(false)) {
        participants << contact->id();
    }

    History::ThreadPtr thread = threadForParticipants(textChannel->property("accountId").toString(),
                                                      History::EventTypeText,
                                                      participants,
                                                      matchFlagsForChannel(textChannel),
                                                      true);
    History::TextEventPtr event = History::ItemFactory::instance()->createTextEvent(thread->accountId(),
                                                                                    thread->threadId(),
                                                                                    messageToken,
                                                                                    "self",
                                                                                    QDateTime::currentDateTime(), // FIXME: check why message.sent() is empty
                                                                                    false, // outgoing messages are never new (unseen)
                                                                                    message.text(),
                                                                                    History::MessageTypeText, // FIXME: add support for MMS
                                                                                    History::MessageFlags(),
                                                                                    QDateTime());
    mWriter->writeTextEvent(event);
}

History::MatchFlags HistoryDaemon::matchFlagsForChannel(const Tp::ChannelPtr &channel)
{
    QString protocol = channel->connection()->protocolName();
    if (mProtocolFlags.contains(protocol)) {
        return mProtocolFlags[protocol];
    }

    // default to this value
    return History::MatchCaseSensitive;
}
