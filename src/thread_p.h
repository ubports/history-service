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

#ifndef HISTORY_THREAD_P_H
#define HISTORY_THREAD_P_H

#include <QString>
#include "types.h"

namespace History
{

class Thread;

class ThreadPrivate
{
public:
    explicit ThreadPrivate();
    ThreadPrivate(const QString &theAccountId,
                         const QString &theThreadId,
                         EventType theType,
                         const QStringList &theParticipants,
                         const Event &theLastEvent,
                         int theCount,
                         int theUnreadCount,
                         const Threads &theGroupedThreads);
    virtual ~ThreadPrivate();

    QString accountId;
    QString threadId;
    QStringList participants;
    EventType type;
    Event lastEvent;
    int count;
    int unreadCount;
    Threads groupedThreads;
};

}

#endif // HISTORY_THREAD_P_H
