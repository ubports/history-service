/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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

#include "thread.h"
#include "thread_p.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDebug>
#include <QDBusMetaType>

Q_DECLARE_METATYPE(QList< QVariantMap >)

namespace History
{

// ------------- ThreadPrivate ------------------------------------------------

ThreadPrivate::ThreadPrivate()
{
}

ThreadPrivate::ThreadPrivate(const QString &theAccountId,
                             const QString &theThreadId, EventType theType,
                             const Participants &theParticipants,
                             const QDateTime &theTimestamp,
                             const Event &theLastEvent,
                             int theCount,
                             int theUnreadCount,
                             const Threads &theGroupedThreads,
                             ChatType theChatType,
                             const QVariantMap &theChatRoomInfo) :
    accountId(theAccountId), threadId(theThreadId), type(theType), participants(theParticipants), timestamp(theTimestamp),
    lastEvent(theLastEvent), count(theCount), unreadCount(theUnreadCount), groupedThreads(theGroupedThreads),
    chatType(theChatType), chatRoomInfo(theChatRoomInfo)
{
}

ThreadPrivate::~ThreadPrivate()
{
}

// ------------- Thread ------------------------------------------------------

Thread::Thread()
    : d_ptr(new ThreadPrivate())
{
}

Thread::Thread(const QString &accountId,
               const QString &threadId, EventType type,
               const Participants &participants,
               const QDateTime &timestamp,
               const Event &lastEvent,
               int count,
               int unreadCount,
               const Threads &groupedThreads,
               ChatType chatType,
               const QVariantMap &chatRoomInfo)
: d_ptr(new ThreadPrivate(accountId, threadId, type, participants, timestamp, lastEvent, count, unreadCount, groupedThreads, chatType, chatRoomInfo))
{
    qDBusRegisterMetaType<QList<QVariantMap> >();
    qRegisterMetaType<QList<QVariantMap> >();
}

Thread::Thread(const Thread &other)
    : d_ptr(new ThreadPrivate(*other.d_ptr))
{
}

Thread::~Thread()
{
}

Thread &Thread::operator=(const Thread &other)
{
    if (&other == this) {
        return *this;
    }
    d_ptr = QSharedPointer<ThreadPrivate>(new ThreadPrivate(*other.d_ptr));
    return *this;
}

QString Thread::accountId() const
{
    Q_D(const Thread);
    return d->accountId;
}

QString Thread::threadId() const
{
    Q_D(const Thread);
    return d->threadId;
}

EventType Thread::type() const
{
    Q_D(const Thread);
    return d->type;
}

Participants Thread::participants() const
{
    Q_D(const Thread);
    return d->participants;
}

QDateTime Thread::timestamp() const
{
    Q_D(const Thread);
    return d->timestamp;
}

Event Thread::lastEvent() const
{
    Q_D(const Thread);
    return d->lastEvent;
}

int Thread::count() const
{
    Q_D(const Thread);
    return d->count;
}

int Thread::unreadCount() const
{
    Q_D(const Thread);
    return d->unreadCount;
}

History::Threads Thread::groupedThreads() const
{
    Q_D(const Thread);
    return d->groupedThreads;
}

ChatType Thread::chatType() const
{
    Q_D(const Thread);
    return d->chatType;
}

QVariantMap Thread::chatRoomInfo() const
{
    Q_D(const Thread);
    return d->chatRoomInfo;
}

bool Thread::isNull() const
{
    Q_D(const Thread);
    return d->accountId.isNull() && d->threadId.isNull() && d->participants.isEmpty();
}

bool Thread::operator ==(const Thread &other) const
{
    Q_D(const Thread);
    if (d->type != other.d_ptr->type) {
        return false;
    }
    if (d->accountId != other.d_ptr->accountId) {
        return false;
    }
    if (d->threadId != other.d_ptr->threadId) {
        return false;
    }
    return true;
}

bool Thread::operator<(const Thread &other) const
{
    QString selfData = QString::number(type()) + accountId() + threadId();
    QString otherData = QString::number(other.type()) + other.accountId() + other.threadId();
    return selfData < otherData;
}

void Thread::removeParticipants(const Participants &participants)
{
    Q_D(Thread);
    Q_FOREACH(const Participant &participant, participants) {
        d->participants.removeAll(participant);
    }
}

void Thread::addParticipants(const Participants &participants)
{
    Q_D(Thread);
    Q_FOREACH(const Participant &participant, participants) {
        d->participants.append(participant);
    }
}

QVariantMap Thread::properties() const
{
    Q_D(const Thread);

    if (d->accountId.isEmpty() || d->threadId.isEmpty()) {
        return QVariantMap();
    }

    // include the properties from the last event
    QVariantMap map = lastEvent().properties();

    // and add the thread ones too
    map[FieldAccountId] = d->accountId;
    map[FieldThreadId] = d->threadId;
    map[FieldType] = d->type;
    map[FieldChatType] = d->chatType;
    map[FieldParticipants] = d->participants.toVariantList();
    map[FieldTimestamp] = d->timestamp;
    map[FieldCount] = d->count;
    map[FieldUnreadCount] = d->unreadCount;
    map[FieldLastEventId] = lastEvent().eventId();
    map[FieldLastEventTimestamp] = d->timestamp;
    map[FieldChatRoomInfo] = d->chatRoomInfo;

    QList<QVariantMap> groupedThreads;
    Q_FOREACH(const Thread &thread, d->groupedThreads) {
        groupedThreads << thread.properties();
    }
    if (!groupedThreads.isEmpty()) {
        map[FieldGroupedThreads] = QVariant::fromValue(groupedThreads);
    }

    return map;
}

Thread Thread::fromProperties(const QVariantMap &properties)
{
    Thread thread;
    if (properties.isEmpty()) {
        return thread;
    }

    // FIXME: save the rest of the data
    QString accountId = properties[FieldAccountId].toString();
    QString threadId = properties[FieldThreadId].toString();
    EventType type = (EventType) properties[FieldType].toInt();
    ChatType chatType = (ChatType) properties[FieldChatType].toInt();
    Participants participants = Participants::fromVariant(properties[FieldParticipants]);
    QDateTime timestamp = QDateTime::fromString(properties[FieldTimestamp].toString(), Qt::ISODate);
    int count = properties[FieldCount].toInt();
    int unreadCount = properties[FieldUnreadCount].toInt();

    Threads groupedThreads;
    if (properties.contains(FieldGroupedThreads)) { 
        QVariant variant = properties[FieldGroupedThreads];
        if (variant.canConvert<QVariantList>()) {
            Q_FOREACH(const QVariant& entry, variant.toList()) {
                groupedThreads << Thread::fromProperties(entry.toMap());
            }
        } else if (variant.canConvert<QDBusArgument>()) {
            QDBusArgument argument = variant.value<QDBusArgument>();
            argument >> groupedThreads;
        }
    }
    QVariantMap chatRoomInfo = qdbus_cast<QVariantMap>(properties[FieldChatRoomInfo]);
    // dbus_cast fails if the map was generated by a qml app, so we demarshal it by hand
    if (chatRoomInfo.isEmpty()) {
        chatRoomInfo = properties[FieldChatRoomInfo].toMap();
    }

    Event event;
    switch (type) {
        case EventTypeText:
            event = TextEvent::fromProperties(properties);
            break;
        case EventTypeVoice:
            event = VoiceEvent::fromProperties(properties);
            break;
    }
    return Thread(accountId, threadId, type, participants, timestamp, event, count, unreadCount, groupedThreads, chatType, chatRoomInfo);
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Threads &threads)
{
    argument.beginArray();
    while (!argument.atEnd()) {
        QVariantMap props;
        QVariant variant;
        argument >> variant;
        QDBusArgument innerArgument = variant.value<QDBusArgument>();
        if (!innerArgument.atEnd()) {
            innerArgument >> props;
        }
        threads << Thread::fromProperties(props);
    }
    argument.endArray();
    return argument;
}

}
