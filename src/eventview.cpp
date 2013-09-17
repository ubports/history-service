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
#include "itemfactory.h"
#include "manager.h"
#include "sort.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDBusInterface>
#include <QDBusReply>

namespace History
{

// ------------- EventViewPrivate ------------------------------------------------

EventViewPrivate::EventViewPrivate(History::EventType theType,
                                   const History::Sort &theSort,
                                   const History::Filter &theFilter)
    : type(theType), sort(theSort), filter(theFilter), valid(true), dbus(0)
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
        if (event->type() == type && filter.match(event->properties())) {
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

EventView::EventView(EventType type, const History::Sort &sort, const History::Filter &filter)
    : d_ptr(new EventViewPrivate(type, sort, filter))
{
    d_ptr->q_ptr = this;

    QDBusInterface interface(History::DBusService, History::DBusObjectPath, History::DBusInterface);

    QDBusReply<QString> reply = interface.call("QueryEvents",
                                               (int) type,
                                               sort.properties(),
                                               filter.toString());
    if (!reply.isValid()) {
        Q_EMIT invalidated();
        d_ptr->valid = false;
        return;
    }

    d_ptr->objectPath = reply.value();
    d_ptr->dbus = new QDBusInterface(History::DBusService, d_ptr->objectPath, History::EventViewInterface,
                                     QDBusConnection::sessionBus(), this);

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
    Q_D(EventView);
    if (d->valid) {
        d->dbus->call("Destroy");
    }
}

Events EventView::nextPage()
{
    Q_D(EventView);
    Events events;

    if (!d->valid) {
        return events;
    }

    QDBusReply<QList<QVariantMap> > reply = d->dbus->call("NextPage");

    if (!reply.isValid()) {
        d->valid = false;
        Q_EMIT invalidated();
        return events;
    }

    QList<QVariantMap> eventsProperties = reply.value();
    Q_FOREACH(const QVariantMap &properties, eventsProperties) {
        EventPtr event;
        switch (d->type) {
        case EventTypeText:
            event = ItemFactory::instance()->createTextEvent(properties);
            break;
        case EventTypeVoice:
            event = ItemFactory::instance()->createVoiceEvent(properties);
            break;
        }

        if (!event.isNull()) {
            events << event;
        }
    }

    return events;
}

bool EventView::isValid() const
{
    Q_D(const EventView);
    return d->valid;
}

}

#include "moc_eventview.cpp"
