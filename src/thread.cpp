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

namespace History
{

// ------------- ThreadPrivate ------------------------------------------------

ThreadPrivate::ThreadPrivate(const QString &theAccountId,
                                           const QString &theThreadId, EventType theType,
                                           const QStringList &theParticipants,
                                           const EventPtr &theLastEvent,
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
               const EventPtr &lastEvent,
               int count,
               int unreadCount)
: d_ptr(new ThreadPrivate(accountId, threadId, type, participants, lastEvent, count, unreadCount))
{
}

Thread::~Thread()
{
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

EventPtr Thread::lastEvent() const
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

QVariantMap Thread::properties() const
{
    Q_D(const Thread);

    QVariantMap map;
    map["accountId"] = d->accountId;
    map["threadId"] = d->threadId;
    map["type"] = d->type;
    map["participants"] = d->participants;
    map["count"] = d->count;
    map["unreadCount"] = d->unreadCount;

    return map;
}

}
