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

#include "threadview.h"
#include "threadview_p.h"
#include "filter.h"
#include "manager.h"
#include "thread.h"

namespace History
{

// ------------- ThreadViewPrivate ------------------------------------------------

ThreadViewPrivate::ThreadViewPrivate(History::EventType theType,
                                     const History::SortPtr &theSort,
                                     const History::FilterPtr &theFilter)
    : type(theType), sort(theSort), filter(theFilter)
{
}

Threads ThreadViewPrivate::filteredThreads(const Threads &threads)
{
    // if the filter is null, return all threads
    if (filter.isNull()) {
        return threads;
    }

    Threads filtered;
    Q_FOREACH(const ThreadPtr &thread, threads) {
        if (filter->match(thread->properties())) {
            filtered << thread;
        }
    }

    return filtered;
}

void ThreadViewPrivate::_d_threadsAdded(const History::Threads &threads)
{
    Q_Q(ThreadView);

    Threads filtered = filteredThreads(threads);
    if (!filtered.isEmpty()) {
        Q_EMIT q->threadsAdded(filtered);
    }
}

void ThreadViewPrivate::_d_threadsModified(const Threads &threads)
{
    Q_Q(ThreadView);

    Threads filtered = filteredThreads(threads);
    if (!filtered.isEmpty()) {
        Q_EMIT q->threadsModified(filtered);
    }
}

void ThreadViewPrivate::_d_threadsRemoved(const Threads &threads)
{
    Q_Q(ThreadView);

    Threads filtered = filteredThreads(threads);
    if (!filtered.isEmpty()) {
        Q_EMIT q->threadsRemoved(filtered);
    }
}

// ------------- ThreadView -------------------------------------------------------

ThreadView::ThreadView(History::EventType type,
                       const History::SortPtr &sort,
                       const History::FilterPtr &filter)
    : d_ptr(new ThreadViewPrivate(type, sort, filter))
{
    d_ptr->q_ptr = this;

    connect(Manager::instance(),
            SIGNAL(threadsAdded(History::Threads)),
            SLOT(_d_threadsAdded(History::Threads)));
    connect(Manager::instance(),
            SIGNAL(threadsModified(History::Threads)),
            SLOT(_d_threadsModified(History::Threads)));
    connect(Manager::instance(),
            SIGNAL(threadsRemoved(History::Threads)),
            SLOT(_d_threadsRemoved(History::Threads)));
}

ThreadView::~ThreadView()
{
}

}

#include "moc_threadview.cpp"
