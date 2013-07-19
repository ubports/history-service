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

#include "eventwatcher_p.h"
#include "manager.h"
#include <QDBusConnection>
#include <QDebug>

#define HISTORY_INTERFACE "com.canonical.HistoryService"

namespace History
{

EventWatcher::EventWatcher(QObject *parent) :
    QObject(parent)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.connect(QString::null, QString::null, HISTORY_INTERFACE, "ThreadsAdded",
                       this, SLOT(onThreadsAdded(QList<QVariantMap>)));
}

EventWatcher *EventWatcher::instance()
{
    static EventWatcher *self = new EventWatcher();
    return self;
}

void EventWatcher::onThreadsAdded(const QList<QVariantMap> &threads)
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT threadsAdded(threadsFromProperties(threads));
}

void EventWatcher::onThreadsModified(const QList<QVariantMap> &threads)
{
    Q_EMIT threadsModified(threadsFromProperties(threads));
}

void EventWatcher::onThreadsRemoved(const QList<QVariantMap> &threads)
{
    Q_EMIT threadsRemoved(threadsFromProperties(threads));
}

void EventWatcher::onEventsAdded(const QList<QVariantMap> &events)
{
    Q_EMIT eventsAdded(eventsFromProperties(events));
}

void EventWatcher::onEventsModified(const QList<QVariantMap> &events)
{
    Q_EMIT eventsAdded(eventsFromProperties(events));
}

void EventWatcher::onEventsRemoved(const QList<QVariantMap> &events)
{
    Q_EMIT eventsAdded(eventsFromProperties(events));
}

Threads EventWatcher::threadsFromProperties(const QList<QVariantMap> &threadsProperties)
{
    Threads threads;
    Manager *manager = Manager::instance();

    Q_FOREACH(const QVariantMap &map, threadsProperties) {
        QString accountId = map["accountId"].toString();
        QString threadId = map["threadId"].toString();
        EventType type = (EventType) map["type"].toInt();
        ThreadPtr thread = manager->getSingleThread(type, accountId, threadId);
        if (!thread.isNull()) {
            threads << thread;
        }
    }

    return threads;
}

Events EventWatcher::eventsFromProperties(const QList<QVariantMap> &eventsProperties)
{
}

}
