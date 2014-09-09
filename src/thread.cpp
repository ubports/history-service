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

#include "thread.h"
#include "thread_p.h"
#include "textevent.h"
#include "voiceevent.h"

namespace History
{

// ------------- ThreadPrivate ------------------------------------------------

ThreadPrivate::ThreadPrivate()
{
}

ThreadPrivate::ThreadPrivate(const QString &theAccountId,
                                           const QString &theThreadId, EventType theType,
                                           const QStringList &theParticipants,
                                           const Event &theLastEvent,
                                           int theCount,
                                           int theUnreadCount) :
    accountId(theAccountId), threadId(theThreadId), type(theType), participants(theParticipants),
    lastEvent(theLastEvent), count(theCount), unreadCount(theUnreadCount)
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
               const QStringList &participants,
               const Event &lastEvent,
               int count,
               int unreadCount)
: d_ptr(new ThreadPrivate(accountId, threadId, type, participants, lastEvent, count, unreadCount))
{
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

QStringList Thread::participants() const
{
    Q_D(const Thread);
    return d->participants;
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

QVariantMap Thread::properties() const
{
    Q_D(const Thread);

    QVariantMap map;
    map[FieldAccountId] = d->accountId;
    map[FieldThreadId] = d->threadId;
    map[FieldType] = d->type;
    map[FieldParticipants] = d->participants;
    map[FieldCount] = d->count;
    map[FieldUnreadCount] = d->unreadCount;
    map[FieldLastEventId] = lastEvent().eventId();
    map[FieldLastEventTimestamp] = lastEvent().timestamp();

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
    QStringList participants = properties[FieldParticipants].toStringList();
    int count = properties[FieldCount].toInt();
    int unreadCount = properties[FieldUnreadCount].toInt();

    Event event;
    switch (type) {
        case EventTypeText:
            event = TextEvent::fromProperties(properties);
            break;
        case EventTypeVoice:
            event = VoiceEvent::fromProperties(properties);
            break;
    }
    return Thread(accountId, threadId, type, participants, event, count, unreadCount);
}

}
