/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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
#include "telepathyhelper_p.h"
#include "filter.h"
#include "sort.h"

#include "pluginmanager.h"
#include "plugin.h"
#include "pluginthreadview.h"
#include "plugineventview.h"

#include <QStandardPaths>
#include <QCryptographicHash>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/ReferencedHandles>

HistoryDaemon::HistoryDaemon(QObject *parent)
    : QObject(parent), mCallObserver(this), mTextObserver(this)
{
    // get the first plugin
    if (!History::PluginManager::instance()->plugins().isEmpty()) {
        mBackend = History::PluginManager::instance()->plugins().first();
    }

    // FIXME: maybe we should only set the plugin as ready after the contact cache was generated
    connect(TelepathyHelper::instance(), &TelepathyHelper::setupReady, [&]() {
        mBackend->generateContactCache();
    });

    connect(TelepathyHelper::instance(),
            SIGNAL(channelObserverCreated(ChannelObserver*)),
            SLOT(onObserverCreated()));
    TelepathyHelper::instance()->registerChannelObserver();

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
    mProtocolFlags["multimedia"] = History::MatchPhoneNumber;

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

QStringList HistoryDaemon::participantsFromChannel(const Tp::TextChannelPtr &textChannel)
{
    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupContacts(false)) {
        participants << contact->id();
    }

    if (participants.isEmpty() && textChannel->targetHandleType() == Tp::HandleTypeContact &&
           textChannel->targetContact() == textChannel->connection()->selfContact()) {
        participants << textChannel->targetContact()->id();
    }
    return participants;
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

    waitForBackendInitialised();

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

QString HistoryDaemon::queryThreads(int type, const QVariantMap &sort, const QVariantMap &filter, const QVariantMap &properties)
{
    if (!mBackend) {
        return QString::null;
    }

    waitForBackendInitialised();

    History::Sort theSort = History::Sort::fromProperties(sort);
    History::Filter theFilter = History::Filter::fromProperties(filter);
    History::PluginThreadView *view = mBackend->queryThreads((History::EventType)type, theSort, theFilter, properties);

    if (!view) {
        return QString::null;
    }

    // FIXME: maybe we should keep a list of views to manually remove them at some point?
    view->setParent(this);
    return view->objectPath();
}

QString HistoryDaemon::queryEvents(int type, const QVariantMap &sort, const QVariantMap &filter)
{
    if (!mBackend) {
        return QString::null;
    }

    waitForBackendInitialised();

    History::Sort theSort = History::Sort::fromProperties(sort);
    History::Filter theFilter = History::Filter::fromProperties(filter);
    History::PluginEventView *view = mBackend->queryEvents((History::EventType)type, theSort, theFilter);

    if (!view) {
        return QString::null;
    }

    // FIXME: maybe we should keep a list of views to manually remove them at some point?
    view->setParent(this);
    return view->objectPath();
}

QVariantMap HistoryDaemon::getSingleThread(int type, const QString &accountId, const QString &threadId, const QVariantMap &properties)
{
    if (!mBackend) {
        return QVariantMap();
    }

    waitForBackendInitialised();

    return mBackend->getSingleThread((History::EventType)type, accountId, threadId, properties);
}

QVariantMap HistoryDaemon::getSingleEvent(int type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    if (!mBackend) {
        return QVariantMap();
    }

    waitForBackendInitialised();

    return mBackend->getSingleEvent((History::EventType)type, accountId, threadId, eventId);
}

bool HistoryDaemon::writeEvents(const QList<QVariantMap> &events)
{
    if (!mBackend) {
        return false;
    }

    waitForBackendInitialised();

    QList<QVariantMap> newEvents;
    QList<QVariantMap> modifiedEvents;
    QMap<QString, QVariantMap> threads;

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
        QVariantMap thread = getSingleThread(type, accountId, threadId, QVariantMap());
        QString hash = hashThread(thread);
        threads[hash] = thread;

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
        mDBus.notifyThreadsModified(threads.values());
    }
    return true;
}

bool HistoryDaemon::removeEvents(const QList<QVariantMap> &events)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!mBackend) {
        return false;
    }

    waitForBackendInitialised();

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
    QMap<QString, QVariantMap> removedThreads;
    QMap<QString, QVariantMap> modifiedThreads;
    Q_FOREACH(const QVariantMap &event, events) {
         History::EventType type = (History::EventType) event[History::FieldType].toInt();
         QString accountId = event[History::FieldAccountId].toString();
         QString threadId = event[History::FieldThreadId].toString();

         QVariantMap thread = mBackend->getSingleThread(type, accountId, threadId, QVariantMap());
         if (thread.isEmpty()) {
             continue;
         }

         QString hash = hashThread(thread);
         if (thread[History::FieldCount].toInt() > 0) {
             // the thread still has items and we should notify it was modified
             modifiedThreads[hash] = thread;
         } else {
             removedThreads[hash] = thread;
         }
    }

    // finally remove the threads that are now empty
    Q_FOREACH(const QVariantMap &thread, removedThreads.values()) {
        // the thread is now empty and needs to be removed
        if (!mBackend->removeThread(thread)) {
            mBackend->rollbackBatchOperation();
            return false;
        }

    }

    mBackend->endBatchOperation();

    mDBus.notifyEventsRemoved(events);
    if (!removedThreads.isEmpty()) {
        mDBus.notifyThreadsRemoved(removedThreads.values());
    }
    if (!modifiedThreads.isEmpty()) {
        mDBus.notifyThreadsModified(modifiedThreads.values());
    }
    return true;
}

bool HistoryDaemon::removeThreads(const QList<QVariantMap> &threads)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!mBackend) {
        return false;
    }

    waitForBackendInitialised();

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

    // it shouldn't happen, but in case it does, we won't crash
    if (participants.isEmpty()) {
        qWarning() << "Participants list was empty for call channel" << channel;
        return;
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
    int duration;
    bool missed = incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer;

    if (!missed) {
        QDateTime activeTime = channel->property("activeTimestamp").toDateTime();
        duration = activeTime.secsTo(QDateTime::currentDateTime());
    }

    QString eventId = QString("%1:%2").arg(thread[History::FieldThreadId].toString()).arg(timestamp.toString());
    QVariantMap event;
    event[History::FieldType] = History::EventTypeVoice;
    event[History::FieldAccountId] = thread[History::FieldAccountId];
    event[History::FieldThreadId] = thread[History::FieldThreadId];
    event[History::FieldEventId] = eventId;
    event[History::FieldSenderId] = incoming ? channel->initiatorContact()->id() : "self";
    event[History::FieldTimestamp] = timestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz");
    event[History::FieldNewEvent] = missed; // only mark as a new (unseen) event if it is a missed call
    event[History::FieldMissed] = missed;
    event[History::FieldDuration] = duration;
    // FIXME: check what to do when there are more than just one remote participant
    event[History::FieldRemoteParticipant] = participants[0];
    writeEvents(QList<QVariantMap>() << event);
}

void HistoryDaemon::onMessageReceived(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString eventId;
    Tp::MessagePart header = message.header();
    QString senderId;
    History::MessageStatus status = History::MessageStatusUnknown;
    if (message.sender()->handle().at(0) == textChannel->connection()->selfHandle()) {
        senderId = "self";
        status = History::MessageStatusDelivered;
    } else {
        senderId = message.sender()->id();
    }
    if (message.messageToken().isEmpty()) {
        eventId = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    } else {
        eventId = message.messageToken();
    }
 
    // ignore delivery reports for now.
    // FIXME: maybe we should set the readTimestamp when a delivery report is received
    if (message.isRescued()) {
        return;
    }

    if (message.isDeliveryReport() && message.deliveryDetails().hasOriginalToken()) {
        // at this point we assume the delivery report is for a message that was already
        // sent and properly saved at our database, so we can safely get it here to update
        QStringList participants = participantsFromChannel(textChannel);

        QVariantMap thread = threadForParticipants(textChannel->property(History::FieldAccountId).toString(),
                                                                         History::EventTypeText,
                                                                         participants,
                                                                         matchFlagsForChannel(textChannel),
                                                                         false);
        if (thread.isEmpty()) {
            qWarning() << "Cound not find the thread related to this delivery report.";
            return;
        }

        QVariantMap textEvent = getSingleEvent((int)History::EventTypeText,
                                               textChannel->property(History::FieldAccountId).toString(),
                                               thread[History::FieldThreadId].toString(),
                                               message.deliveryDetails().originalToken());
        if (textEvent.isEmpty()) {
            qWarning() << "Cound not find the original event to update with delivery details.";
            return;
        }

        History::MessageStatus status;
        switch (message.deliveryDetails().status()) {
        case Tp::DeliveryStatusAccepted:
            status = History::MessageStatusAccepted;
            break;
        case Tp::DeliveryStatusDeleted:
            status = History::MessageStatusDeleted;
            break;
        case Tp::DeliveryStatusDelivered:
            status = History::MessageStatusDelivered;
            break;
        case Tp::DeliveryStatusPermanentlyFailed:
            status = History::MessageStatusPermanentlyFailed;
            break;
        case Tp::DeliveryStatusRead:
            status = History::MessageStatusRead;
            break;
        case Tp::DeliveryStatusTemporarilyFailed:
            status = History::MessageStatusTemporarilyFailed;
            break;
        case Tp::DeliveryStatusUnknown:
            status = History::MessageStatusUnknown;
            break;
        }

        textEvent[History::FieldMessageStatus] = (int) status;
        if (!writeEvents(QList<QVariantMap>() << textEvent)) {
            qWarning() << "Failed to save the new message status!";
        }

        return;
    }

    QStringList participants = participantsFromChannel(textChannel);

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
        QString normalizedEventId = QString(QCryptographicHash::hash(eventId.toLatin1(), QCryptographicHash::Md5).toHex());
        QString mmsStoragePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

        type = History::MessageTypeMultiPart;
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
            attachment[History::FieldEventId] = eventId;
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
    event[History::FieldEventId] = eventId;
    event[History::FieldSenderId] = senderId;
    event[History::FieldTimestamp] = message.received().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    event[History::FieldNewEvent] = true; // message is always unread until it reaches HistoryDaemon::onMessageRead
    event[History::FieldMessage] = message.text();
    event[History::FieldMessageType] = (int)type;
    event[History::FieldMessageStatus] = (int)status;
    event[History::FieldReadTimestamp] = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
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
    QStringList participants = participantsFromChannel(textChannel);
    QList<QVariantMap> attachments;
    History::MessageType type = History::MessageTypeText;
    int count = 1;
    QString subject;
    QString eventId;

    if (messageToken.isEmpty()) {
        eventId = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    } else {
        eventId = messageToken;
    }
 
    QVariantMap thread = threadForParticipants(textChannel->property(History::FieldAccountId).toString(),
                                              History::EventTypeText,
                                              participants,
                                              matchFlagsForChannel(textChannel),
                                              true);
    if (message.hasNonTextContent()) {
        QString normalizedAccountId = QString(QCryptographicHash::hash(thread[History::FieldAccountId].toString().toLatin1(), QCryptographicHash::Md5).toHex());
        QString normalizedThreadId = QString(QCryptographicHash::hash(thread[History::FieldThreadId].toString().toLatin1(), QCryptographicHash::Md5).toHex());
        QString normalizedEventId = QString(QCryptographicHash::hash(eventId.toLatin1(), QCryptographicHash::Md5).toHex());
        QString mmsStoragePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

        type = History::MessageTypeMultiPart;
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
            attachment[History::FieldEventId] = eventId;
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
    event[History::FieldEventId] = eventId;
    event[History::FieldSenderId] = "self";
    event[History::FieldTimestamp] = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz"); // FIXME: check why message.sent() is empty
    event[History::FieldNewEvent] =  false; // outgoing messages are never new (unseen)
    event[History::FieldMessage] = message.text();
    event[History::FieldMessageType] = type;
    if (textChannel->deliveryReportingSupport() & Tp::DeliveryReportingSupportFlagReceiveSuccesses) {
        event[History::FieldMessageStatus] = (int)History::MessageStatusUnknown;
    } else {
        event[History::FieldMessageStatus] = (int)History::MessageStatusAccepted;
    }
    event[History::FieldReadTimestamp] = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    event[History::FieldSubject] = "";
    event[History::FieldAttachments] = QVariant::fromValue(attachments);

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

QString HistoryDaemon::hashThread(const QVariantMap &thread)
{
    QString hash = QString::number(thread[History::FieldType].toInt());
    hash += "#-#" + thread[History::FieldAccountId].toString();
    hash += "#-#" + thread[History::FieldThreadId].toString();
    return hash;
}
