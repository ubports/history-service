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

#ifndef THREADVIEW_P_H
#define THREADVIEW_P_H

#include "types.h"
#include <QDBusInterface>

namespace History
{
    class ThreadView;

    class ThreadViewPrivate
    {
        Q_DECLARE_PUBLIC(ThreadView)

    public:
        ThreadViewPrivate(History::EventType theType,
                          const History::Sort &theSort,
                          const History::Filter &theFilter);
        EventType type;
        Sort sort;
        Filter filter;
        QString objectPath;
        bool valid;
        QDBusInterface *dbus;

        Threads filteredThreads(const Threads &threads);

        // private slots
        void _d_threadsAdded(const History::Threads &threads);
        void _d_threadsModified(const History::Threads &threads);
        void _d_threadsRemoved(const History::Threads &threads);
        void _d_threadParticipantsChanged(const History::Thread &thread,
                                   const History::Participants &added,
                                   const History::Participants &removed,
                                   const History::Participants &modified);

        ThreadView *q_ptr;
    };
}

#endif // THREADVIEW_P_H
