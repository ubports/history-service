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

#include "manager.h"
#include "manager_p.h"
#include "managerdbus_p.h"
#include "eventview.h"
#include "intersectionfilter.h"
#include "itemfactory.h"
#include "textevent.h"
#include "thread.h"
#include "threadview.h"
#include "voiceevent.h"
#include <QDebug>

#define HISTORY_INTERFACE "com.canonical.HistoryService"

namespace History
{

// ------------- ManagerPrivate ------------------------------------------------

ManagerPrivate::ManagerPrivate()
    : dbus(new ManagerDBus())
{
}

ManagerPrivate::~ManagerPrivate()
{
}

// ------------- Manager -------------------------------------------------------

Manager::Manager()
    : d_ptr(new ManagerPrivate())
{
    Q_D(Manager);

    // Propagate the signals from the event watcher
    connect(d->dbus.data(),
            SIGNAL(threadsAdded(History::Threads)),
            SIGNAL(threadsAdded(History::Threads)));
    connect(d->dbus.data(),
            SIGNAL(threadsModified(History::Threads)),
            SIGNAL(threadsModified(History::Threads)));
    connect(d->dbus.data(),
            SIGNAL(threadsRemoved(History::Threads)),
            SIGNAL(threadsRemoved(History::Threads)));
    connect(d->dbus.data(),
            SIGNAL(eventsAdded(History::Events)),
            SIGNAL(eventsAdded(History::Events)));
    connect(d->dbus.data(),
            SIGNAL(eventsModified(History::Events)),
            SIGNAL(eventsModified(History::Events)));
    connect(d->dbus.data(),
            SIGNAL(eventsRemoved(History::Events)),
            SIGNAL(eventsRemoved(History::Events)));
}

Manager::~Manager()
{
}

Manager *Manager::instance()
{
    static Manager *self = new Manager();
    return self;
}

ThreadViewPtr Manager::queryThreads(EventType type,
                                    const SortPtr &sort,
                                    const FilterPtr &filter)
{
    return ThreadViewPtr(new ThreadView(type, sort, filter));
}

EventViewPtr Manager::queryEvents(EventType type,
                                  const SortPtr &sort,
                                  const FilterPtr &filter)
{
    return EventViewPtr(new EventView(type, sort, filter));
}

EventPtr Manager::getSingleEvent(EventType type, const QString &accountId, const QString &threadId, const QString &eventId, bool useCache)
{
    Q_D(Manager);

    EventPtr event;
    if (useCache) {
        event = ItemFactory::instance()->cachedEvent(accountId, threadId, eventId, type);
    }

    if (event.isNull()) {
        event = d->dbus->getSingleEvent(type, accountId, threadId, eventId);
    }

    return event;
}

ThreadPtr Manager::threadForParticipants(const QString &accountId,
                                         EventType type,
                                         const QStringList &participants,
                                         MatchFlags matchFlags,
                                         bool create)
{
    Q_D(Manager);

    return d->dbus->threadForParticipants(accountId, type, participants, matchFlags, create);
}

ThreadPtr Manager::getSingleThread(EventType type, const QString &accountId, const QString &threadId, bool useCache)
{
    Q_D(Manager);

    ThreadPtr thread;
    if (useCache) {
        thread = ItemFactory::instance()->cachedThread(accountId, threadId, type);
    }

    if (thread.isNull()) {
        thread = d->dbus->getSingleThread(type, accountId, threadId);
    }
    return thread;
}

bool Manager::writeEvents(const Events &events)
{
    Q_D(Manager);
    return d->dbus->writeEvents(events);
}

bool Manager::removeThreads(const Threads &threads)
{
    Q_D(Manager);
    return d->dbus->removeThreads(threads);
}

bool Manager::removeEvents(const Events &events)
{
    Q_D(Manager);
    return d->dbus->removeEvents(events);
}

}

