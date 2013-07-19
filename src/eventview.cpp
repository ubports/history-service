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

#include "eventview.h"
#include "eventview_p.h"
#include "event.h"
#include "filter.h"
#include "manager.h"

namespace History
{

// ------------- EventViewPrivate ------------------------------------------------

EventViewPrivate::EventViewPrivate(History::EventType theType,
                                   const History::SortPtr &theSort,
                                   const History::FilterPtr &theFilter)
    : type(theType), sort(theSort), filter(theFilter)
{
}

Events EventViewPrivate::filteredEvents(const Events &events)
{
    // if the filter is null, return all threads
    if (filter.isNull()) {
        return events;
    }

    Events filtered;
    Q_FOREACH(const EventPtr &event, events) {
        if (filter->match(event->properties())) {
            filtered << events;
        }
    }

    return filtered;
}

void EventViewPrivate::_d_eventsAdded(const Events &events)
{
    Q_Q(EventView);

    Events filtered = filteredEvents(events);
    if (!filtered.isEmpty()) {
        Q_EMIT q->eventsAdded(filtered);
    }
}

void EventViewPrivate::_d_eventsModified(const Events &events)
{
    Q_Q(EventView);

    Events filtered = filteredEvents(events);
    if (!filtered.isEmpty()) {
        Q_EMIT q->eventsModified(filtered);
    }
}

void EventViewPrivate::_d_eventsRemoved(const Events &events)
{
    Q_Q(EventView);

    Events filtered = filteredEvents(events);
    if (!filtered.isEmpty()) {
        Q_EMIT q->eventsRemoved(filtered);
    }
}

// ------------- EventView -------------------------------------------------------

EventView::EventView(EventType type, const SortPtr &sort, const FilterPtr &filter)
    : d_ptr(new EventViewPrivate(type, sort, filter))
{
    d_ptr->q_ptr = this;

    connect(Manager::instance(),
            SIGNAL(eventsAdded(History::Events)),
            SLOT(_d_eventsAdded(History::Events)));
    connect(Manager::instance(),
            SIGNAL(eventsModified(History::Events)),
            SLOT(_d_eventsModified(History::Events)));
    connect(Manager::instance(),
            SIGNAL(eventsRemoved(History::Events)),
            SLOT(_d_eventsRemoved(History::Events)));
}

EventView::~EventView()
{
}

}

#include "moc_eventview.cpp"
