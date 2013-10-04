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
#include "sort.h"

#include "pluginmanager.h"
#include "plugin.h"
#include "pluginthreadview.h"
#include "plugineventview.h"

#include <QStandardPaths>
#include <QCryptographicHash>
#include <TelepathyQt/CallChannel>

HistoryDaemon::HistoryDaemon(QObject *parent)
    : QObject(parent), mCallObserver(this), mTextObserver(this)
{
    // get the first plugin
    if (!History::PluginManager::instance()->plugins().isEmpty()) {
        mBackend = History::PluginManager::instance()->plugins().first();
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

QVariantMap HistoryDaemon::threadForParticipants(const QString &accountId,
                                                 History::EventType type,
                                                 const QStringList &participants,
                                                 History::MatchFlags matchFlags,
                                                 bool create)
{
    if (!mBackend) {
        return QVariantMap();
    }

    QVariantMap thread = mBackend->threadForParticipants(accountId,
                                                         type,
                                                         participants,
                                                         matchFlags);
    if (thread.isEmpty() && create) {
        thread = mBackend->createThreadForParticipants(accountId, type, participants);
        if (!thread.isEmpty()) {
            mDBus.notifyThreadsAdded(QList<QVariantMap>() << thread);
        }
    }
    return thread;
}

QString HistoryDaemon::queryThreads(int type, const QVariantMap &sort, const QString &filter)
{
    if (!mBackend) {
        return QString::null;
    }

    History::Sort theSort = History::Sort::fromProperties(sort);
    History::PluginThreadView *view = mBackend->queryThreads((History::EventType)type, theSort, filter);

    if (!view) {
        return QString::null;
    }

    // FIXME: maybe we should keep a list of views to manually remove them at some point?
    view->setParent(this);
    return view->objectPath();
}

QString HistoryDaemon::queryEvents(int type, const QVariantMap &sort, const QString &filter)
{
    if (!mBackend) {
        return QString::null;
    }

    History::Sort theSort = History::Sort::fromProperties(sort);
    History::PluginEventView *view = mBackend->queryEvents((History::EventType)type, theSort, filter);

    if (!view) {
        return QString::null;
    }

    // FIXME: maybe we should keep a list of views to manually remove them at some point?
    view->setParent(this);
    return view->objectPath();
}

QVariantMap HistoryDaemon::getSingleThread(int type, const QString &accountId, const QString &threadId)
{
    if (!mBackend) {
        return QVariantMap();
    }

    return mBackend->getSingleThread((History::EventType)type, accountId, threadId);
}

QVariantMap HistoryDaemon::getSingleEvent(int type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    if (!mBackend) {
        return QVariantMap();
    }

    return mBackend->getSingleEvent((History::EventType)type, accountId, threadId, eventId);
}

bool HistoryDaemon::writeEvents(const QList<QVariantMap> &events)
{
    if (!mBackend) {
        return false;
    }

    QList<QVariantMap> newEvents;
    QList<QVariantMap> modifiedEvents;
    QList<QVariantMap> threads;

    mBackend->beginBatchOperation();

    Q_FOREACH(const QVariantMap &event, events) {
        History::EventType type = (History::EventType) event[History::FieldType].toInt();
        History::EventWriteResult result;

        // get the threads for the events to notify their modifications
        QString accountId = event[History::FieldAccountId].toString();
        QString threadId = event[History::FieldThreadId].toString();
        QVariantMap savedEvent = event;

        // and finally write the event
        switch (type) {
        case History::EventTypeText:
            result = mBackend->writeTextEvent(savedEvent);
            break;
        case History::EventTypeVoice:
            result = mBackend->writeVoiceEvent(savedEvent);
            break;
        }

        // only get the thread AFTER the event is written to make sure it is up-to-date
        QVariantMap thread = getSingleThread(type, accountId, threadId);
        threads.removeAll(thread);
        threads << thread;

        // set the participants field in the event
        savedEvent[History::FieldParticipants] = thread[History::FieldParticipants];


        // check if the event was a new one or a modification to an existing one
        switch (result) {
        case History::EventWriteCreated:
            newEvents << savedEvent;
            break;
        case History::EventWriteModified:
            modifiedEvents << savedEvent;
            break;
        case History::EventWriteError:
            mBackend->rollbackBatchOperation();
            return false;
        }
    }

    mBackend->endBatchOperation();

    // and last but not least, notify the results
    if (!newEvents.isEmpty()) {
        mDBus.notifyEventsAdded(newEvents);
    }
    if (!modifiedEvents.isEmpty()) {
        mDBus.notifyEventsModified(modifiedEvents);
    }
    if (!threads.isEmpty()) {
        mDBus.notifyThreadsModified(threads);
    }
    return true;
}

bool HistoryDaemon::removeEvents(const QList<QVariantMap> &events)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!mBackend) {
        return false;
    }

    mBackend->beginBatchOperation();

    Q_FOREACH(const QVariantMap &event, events) {
        History::EventType type = (History::EventType) event[History::FieldType].toInt();
        bool success = true;
        switch (type) {
        case History::EventTypeText:
            success = mBackend->removeTextEvent(event);
            break;
        case History::EventTypeVoice:
            success = mBackend->removeVoiceEvent(event);
            break;
        }

        if (!success) {
            mBackend->rollbackBatchOperation();
            return false;
        }
    }

    // now we need to get all the threads that were affected by the removal of events
    // this loop needs to be separate from the item removal loop because we rely on the
    // count property of threads to decide if they were just modified or if they need to
    // be removed.
    QList<QVariantMap> removedThreads;
    QList<QVariantMap> modifiedThreads;
    Q_FOREACH(const QVariantMap &event, events) {
         History::EventType type = (History::EventType) event[History::FieldType].toInt();
         QString accountId = event[History::FieldAccountId].toString();
         QString threadId = event[History::FieldThreadId].toString();

         QVariantMap thread = mBackend->getSingleThread(type, accountId, threadId);
         if (thread.isEmpty()) {
             continue;
         }

         if (thread[History::FieldCount].toInt() > 0 && !modifiedThreads.contains(thread)) {
             // the thread still has items and we should notify it was modified
             modifiedThreads << thread;
         } else if (!removedThreads.contains(thread)) {
             // the thread is now empty and needs to be removed
             if (!mBackend->removeThread(thread)) {
                 mBackend->rollbackBatchOperation();
                 return false;
             }
             removedThreads << thread;
         }
    }

    mBackend->endBatchOperation();

    mDBus.notifyEventsRemoved(events);
    if (!removedThreads.isEmpty()) {
        mDBus.notifyThreadsRemoved(removedThreads);
    }
    if (!modifiedThreads.isEmpty()) {
        mDBus.notifyThreadsModified(modifiedThreads);
    }
    return true;
}

bool HistoryDaemon::removeThreads(const QList<QVariantMap> &threads)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!mBackend) {
        return false;
    }

    // In order to remove a thread all we have to do is to remove all its items
    // then it is going to be removed by removeEvents() once it detects the thread is
    // empty.
    QList<QVariantMap> events;
    Q_FOREACH(const QVariantMap &thread, threads) {
        events += mBackend->eventsForThread(thread);
    }

    return removeEvents(events);
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
    QVariantMap thread = threadForParticipants(accountId,
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

    QString eventId = QString("%1:%2").arg(thread[History::FieldThreadId].toString()).arg(timestamp.toString());
    QVariantMap event;
    event[History::FieldType] = History::EventTypeVoice;
    event[History::FieldAccountId] = thread[History::FieldAccountId];
    event[History::FieldThreadId] = thread[History::FieldThreadId];
    event[History::FieldEventId] = eventId;
    event[History::FieldSenderId] = incoming ? channel->initiatorContact()->id() : "self";
    event[History::FieldTimestamp] = timestamp.toString(Qt::ISODate);
    event[History::FieldNewEvent] = missed; // only mark as a new (unseen) event if it is a missed call
    event[History::FieldMissed] = missed;
    event[History::FieldDuration] = duration;
    writeEvents(QList<QVariantMap>() << event);
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

    QVariantMap thread = threadForParticipants(textChannel->property(History::FieldAccountId).toString(),
                                                                     History::EventTypeText,
                                                                     participants,
                                                                     matchFlagsForChannel(textChannel),
                                                                     true);
    int count = 1;
    QList<QVariantMap> attachments;
    History::MessageType type = History::MessageTypeText;
    QString subject;

    if (message.hasNonTextContent()) {
        QString normalizedAccountId = QString(QCryptographicHash::hash(thread[History::FieldAccountId].toString().toLatin1(), QCryptographicHash::Md5).toHex());
        QString normalizedThreadId = QString(QCryptographicHash::hash(thread[History::FieldThreadId].toString().toLatin1(), QCryptographicHash::Md5).toHex());
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

            QVariantMap attachment;
            attachment[History::FieldAccountId] = thread[History::FieldAccountId];
            attachment[History::FieldThreadId] = thread[History::FieldThreadId];
            attachment[History::FieldEventId] = message.messageToken();
            attachment[History::FieldAttachmentId] = part["identifier"].variant();
            attachment[History::FieldContentType] = part["content-type"].variant();
            attachment[History::FieldFilePath] = file.fileName();
            attachment[History::FieldStatus] = (int) History::AttachmentDownloaded;
            attachments << attachment;
        }
    }

    QVariantMap event;
    event[History::FieldType] = History::EventTypeText;
    event[History::FieldAccountId] = thread[History::FieldAccountId];
    event[History::FieldThreadId] = thread[History::FieldThreadId];
    event[History::FieldEventId] = message.messageToken();
    event[History::FieldSenderId] = message.sender()->id();
    event[History::FieldTimestamp] = message.received().toString(Qt::ISODate);
    event[History::FieldNewEvent] = true; // message is always unread until it reaches HistoryDaemon::onMessageRead
    event[History::FieldMessage] = message.text();
    event[History::FieldMessageType] = (int)type;
    event[History::FieldMessageFlags] = (int)History::MessageFlags();
    event[History::FieldReadTimestamp] = QDateTime().toString(Qt::ISODate);
    event[History::FieldSubject] = subject;
    event[History::FieldAttachments] = QVariant::fromValue(attachments);

    writeEvents(QList<QVariantMap>() << event);
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

   QVariantMap thread = threadForParticipants(textChannel->property(History::FieldAccountId).toString(),
                                              History::EventTypeText,
                                              participants,
                                              matchFlagsForChannel(textChannel),
                                              true);
    QVariantMap event;
    event[History::FieldType] = History::EventTypeText;
    event[History::FieldAccountId] = thread[History::FieldAccountId];
    event[History::FieldThreadId] = thread[History::FieldThreadId];
    event[History::FieldEventId] = messageToken;
    event[History::FieldSenderId] = "self";
    event[History::FieldTimestamp] = QDateTime::currentDateTime().toString(Qt::ISODate); // FIXME: check why message.sent() is empty
    event[History::FieldNewEvent] =  false; // outgoing messages are never new (unseen)
    event[History::FieldMessage] = message.text();
    event[History::FieldMessageType] = (int)History::MessageTypeText; // FIXME: add support for MMS
    event[History::FieldMessageFlags] = (int)History::MessageFlags();
    event[History::FieldReadTimestamp] = QDateTime().toString(Qt::ISODate);
    event[History::FieldSubject] = "";

    writeEvents(QList<QVariantMap>() << event);
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
