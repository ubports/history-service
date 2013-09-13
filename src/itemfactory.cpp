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

#include "itemfactory_p.h"
#include "itemfactory.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"
#include "thread_p.h"
#include "textevent_p.h"
#include "voiceevent_p.h"
#include <QDBusArgument>

namespace History
{

QString ItemFactoryPrivate::hashItem(EventType type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    return QString("item:%1:%2:%3:%4").arg(QString::number((int) type), accountId, threadId, eventId);
}

void ItemFactoryPrivate::cleanupThreads()
{
    QMap<QString, ThreadWeakPtr>::iterator it = threads.begin();
    QMap<QString, ThreadWeakPtr>::iterator end = threads.end();
    while (it != end) {
        if (it.value().isNull()) {
            it = threads.erase(it);
        } else {
            ++it;
        }
    }
}

void ItemFactoryPrivate::cleanupEvents()
{
    QMap<QString, EventWeakPtr>::iterator it = events.begin();
    QMap<QString, EventWeakPtr>::iterator end = events.end();
    while (it != end) {
        if (it.value().isNull()) {
            it = events.erase(it);
        } else {
            ++it;
        }
    }
}

ItemFactory::ItemFactory()
    : d_ptr(new ItemFactoryPrivate())
{
}

ItemFactory *ItemFactory::instance()
{
    static ItemFactory *self = new ItemFactory();
    return self;
}

ThreadPtr ItemFactory::createThread(const QString &accountId,
                                    const QString &threadId,
                                    EventType type,
                                    const QStringList &participants,
                                    const EventPtr &lastEvent,
                                    int count,
                                    int unreadCount)
{
    Q_D(ItemFactory);

    QString hash = d->hashItem(type, accountId, threadId);

    ThreadPtr thread;
    if (d->threads.contains(hash)) {
        thread = d->threads[hash].toStrongRef();
    }

    if (thread.isNull()) {
        thread = ThreadPtr(new Thread(accountId,
                                      threadId,
                                      type,
                                      participants,
                                      lastEvent,
                                      count,
                                      unreadCount));
        d->threads[hash] = thread;
    }

    // take the opportunity to clean the map
    d->cleanupThreads();

    // update the thread data before returning it
    ThreadPrivate *threadD = thread->d_func();
    threadD->participants = participants;
    threadD->lastEvent = lastEvent;
    threadD->count = count;
    threadD->unreadCount = unreadCount;

    return thread;
}

ThreadPtr ItemFactory::createThread(const QVariantMap &properties)
{
    if (properties.isEmpty()) {
        return ThreadPtr();
    }

    // FIXME: save the rest of the data
    QString accountId = properties[FieldAccountId].toString();
    QString threadId = properties[FieldThreadId].toString();
    EventType type = (EventType) properties[FieldType].toInt();
    QStringList participants = properties[FieldParticipants].toStringList();
    int count = properties[FieldCount].toInt();
    int unreadCount = properties[FieldUnreadCount].toInt();

    EventPtr event;
    switch (type) {
        case EventTypeText:
            event = createTextEvent(properties);
            break;
        case EventTypeVoice:
            event = createVoiceEvent(properties);
            break;
    }
    return createThread(accountId, threadId, type, participants, event, count, unreadCount);
}

TextEventPtr ItemFactory::createTextEvent(const QString &accountId,
                                          const QString &threadId,
                                          const QString &eventId,
                                          const QString &senderId,
                                          const QDateTime &timestamp,
                                          bool newEvent,
                                          const QString &message,
                                          MessageType messageType,
                                          MessageFlags messageFlags,
                                          const QDateTime &readTimestamp,
                                          const QString &subject,
                                          const TextEventAttachments &attachments)
{
    Q_D(ItemFactory);

    QString hash = d->hashItem(EventTypeText, accountId, threadId, eventId);

    EventPtr event;
    if (d->events.contains(hash)) {
        event = d->events[hash].toStrongRef();
    }

    if (event.isNull()) {
        event = TextEventPtr(new TextEvent(accountId,
                                           threadId,
                                           eventId,
                                           senderId,
                                           timestamp,
                                           newEvent,
                                           message,
                                           messageType,
                                           messageFlags,
                                           readTimestamp,
                                           subject,
                                           attachments));
        d->events[hash] = event;
    }

    // take the opportunity to clean the map
    d->cleanupEvents();

    // update the event data before returning it
    TextEventPrivate *eventD = event.staticCast<TextEvent>()->d_func();
    // assume that only newEvent, messageFlags and readTimestamp are going to change
    eventD->newEvent = newEvent;
    eventD->messageFlags = messageFlags;
    eventD->readTimestamp = readTimestamp;

    return event.staticCast<TextEvent>();
}

TextEventPtr ItemFactory::createTextEvent(const QVariantMap &properties)
{
    TextEventPtr event;

    QString accountId = properties["accountId"].toString();
    QString threadId = properties["threadId"].toString();
    QString eventId = properties["eventId"].toString();
    QString senderId = properties["senderId"].toString();
    QDateTime timestamp;
    properties["timestamp"].value<QDBusArgument>() >> timestamp;
    bool newEvent = properties["newEvent"].toBool();
    QString message = properties["message"].toString();
    QString subject = properties["subject"].toString();
    MessageType messageType = (MessageType) properties["messageType"].toInt();
    MessageFlags messageFlags = (MessageFlags) properties["messageFlags"].toInt();
    QDateTime readTimestamp;
    properties["readTimestamp"].value<QDBusArgument>() >> readTimestamp;
    event = createTextEvent(accountId, threadId, eventId, senderId, timestamp, newEvent,
                            message, messageType, messageFlags, readTimestamp, subject);
    // FIXME: handle attachments
    return event;
}

VoiceEventPtr ItemFactory::createVoiceEvent(const QString &accountId,
                                            const QString &threadId,
                                            const QString &eventId,
                                            const QString &senderId,
                                            const QDateTime &timestamp,
                                            bool newEvent,
                                            bool missed,
                                            const QTime &duration)
{
    Q_D(ItemFactory);

    QString hash = d->hashItem(EventTypeVoice, accountId, threadId, eventId);

    EventPtr event;
    if (d->events.contains(hash)) {
        event = d->events[hash].toStrongRef();
    }

    if (event.isNull()) {
        event = VoiceEventPtr(new VoiceEvent(accountId,
                                             threadId,
                                             eventId,
                                             senderId,
                                             timestamp,
                                             newEvent,
                                             missed,
                                             duration));
        d->events[hash] = event;
    }

    // take the opportunity to clean the map
    d->cleanupEvents();

    // update the event data before returning it
    VoiceEventPrivate *eventD = event.staticCast<VoiceEvent>()->d_func();
    // assume that only newEvent can change
    eventD->newEvent = newEvent;

    return event.staticCast<VoiceEvent>();
}

VoiceEventPtr ItemFactory::createVoiceEvent(const QVariantMap &properties)
{
    VoiceEventPtr event;

    QString accountId = properties["accountId"].toString();
    QString threadId = properties["threadId"].toString();
    QString eventId = properties["eventId"].toString();
    QString senderId = properties["senderId"].toString();
    QDateTime timestamp;
    properties["timestamp"].value<QDBusArgument>() >> timestamp;
    bool newEvent = properties["newEvent"].toBool();
    bool missed = properties["missed"].toBool();
    QTime duration = QTime(0,0,0).addSecs(properties["duration"].toInt());
    event = createVoiceEvent(accountId, threadId, eventId, senderId, timestamp, newEvent,
                             missed, duration);
    return event;
}

ThreadPtr ItemFactory::cachedThread(const QString &accountId, const QString &threadId, EventType type)
{
    Q_D(ItemFactory);
    return d->threads[d->hashItem(type, accountId, threadId)];
}

EventPtr ItemFactory::cachedEvent(const QString &accountId, const QString &threadId, const QString &eventId, EventType type)
{
    Q_D(ItemFactory);
    return d->events[d->hashItem(type, accountId, threadId, eventId)];
}

}
