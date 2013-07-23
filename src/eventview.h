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

#ifndef HISTORY_EVENTVIEW_H
#define HISTORY_EVENTVIEW_H

#include "types.h"
#include <QObject>

namespace History
{

class EventViewPrivate;

class EventView : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(EventView)
public:
    EventView(History::EventType type,
              const History::SortPtr &sort,
              const History::FilterPtr &filter);
    virtual ~EventView();

    virtual History::Events nextPage() = 0;
    virtual bool isValid() const = 0;

Q_SIGNALS:
    void eventsAdded(const History::Events &events);
    void eventsModified(const History::Events &events);
    void eventsRemoved(const History::Events &events);

private:
    Q_PRIVATE_SLOT(d_func(), void _d_eventsAdded(const History::Events &events))
    Q_PRIVATE_SLOT(d_func(), void _d_eventsModified(const History::Events &events))
    Q_PRIVATE_SLOT(d_func(), void _d_eventsRemoved(const History::Events &events))
    QScopedPointer<EventViewPrivate> d_ptr;
};

}

#endif // HISTORY_EVENTVIEW_H
