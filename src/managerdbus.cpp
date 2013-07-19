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

#include "managerdbus_p.h"
#include "historyserviceadaptor.h"
#include "event.h"
#include "thread.h"

static const char* DBUS_OBJECT_PATH = "/com/canonical/HistoryService";

namespace History
{

ManagerDBus::ManagerDBus(QObject *parent) :
    QObject(parent), mAdaptor(0)
{
    connectToBus();
}

bool ManagerDBus::connectToBus()
{
    if (!mAdaptor) {
        mAdaptor = new HistoryServiceAdaptor(this);
    }

    return QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);
}

void ManagerDBus::notifyThreadsAdded(const Threads &threads)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT ThreadsAdded(threadsToProperties(threads));
}

void ManagerDBus::notifyThreadsModified(const Threads &threads)
{
    Q_EMIT ThreadsModified(threadsToProperties(threads));
}

void ManagerDBus::notifyThreadsRemoved(const Threads &threads)
{
    Q_EMIT ThreadsRemoved(threadsToProperties(threads));
}

void ManagerDBus::notifyEventsAdded(const Events &events)
{
    Q_EMIT EventsAdded(eventsToProperties(events));
}

void ManagerDBus::notifyEventsModified(const Events &events)
{
    Q_EMIT EventsModified(eventsToProperties(events));
}

void ManagerDBus::notifyEventsRemoved(const Events &events)
{
    Q_EMIT EventsRemoved(eventsToProperties(events));
}

QList<QVariantMap> ManagerDBus::threadsToProperties(const Threads &threads)
{
    QList<QVariantMap> threadsPropertyMap;

    Q_FOREACH(const ThreadPtr &thread, threads) {
        threadsPropertyMap << thread->properties();
    }

    return threadsPropertyMap;
}

QList<QVariantMap> ManagerDBus::eventsToProperties(const Events &events)
{
    QList<QVariantMap> eventsPropertyMap;

    Q_FOREACH(const EventPtr &event, events) {
        eventsPropertyMap << event->properties();
    }

    return eventsPropertyMap;
}

}
