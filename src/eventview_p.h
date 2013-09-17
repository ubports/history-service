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

#ifndef EVENTVIEW_P_H
#define EVENTVIEW_P_H

#include "types.h"
#include <QDBusInterface>

namespace History
{
    class EventView;

    class EventViewPrivate
    {
        Q_DECLARE_PUBLIC(EventView)

    public:
        EventViewPrivate(History::EventType theType,
                          const History::SortPtr &theSort,
                          const History::FilterPtr &theFilter);
        EventType type;
        SortPtr sort;
        FilterPtr filter;
        QString objectPath;
        bool valid;
        QDBusInterface *dbus;

        Events filteredEvents(const Events &events);

        // private slots
        void _d_eventsAdded(const History::Events &events);
        void _d_eventsModified(const History::Events &events);
        void _d_eventsRemoved(const History::Events &events);

        EventView *q_ptr;
    };
}

#endif // EVENTVIEW_P_H
