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
#include "itemfactory.h"
#include "manager.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDBusReply>

Q_DECLARE_METATYPE(QList< QVariantMap >)

namespace History
{

ManagerDBus::ManagerDBus(QObject *parent) :
    QObject(parent), mAdaptor(0), mInterface(DBusService,
                                             DBusObjectPath,
                                             DBusInterface)
{
    qDBusRegisterMetaType<QList<QVariantMap> >();

    // listen for signals coming from the bus
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.connect(DBusService, DBusObjectPath, DBusInterface, "ThreadsAdded",
                       this, SLOT(onThreadsAdded(QList<QVariantMap>)));
    connection.connect(DBusService, DBusObjectPath, DBusInterface, "ThreadsModified",
                       this, SLOT(onThreadsModified(QList<QVariantMap>)));
    connection.connect(DBusService, DBusObjectPath, DBusInterface, "ThreadsRemoved",
                       this, SLOT(onThreadsRemoved(QList<QVariantMap>)));

    connection.connect(DBusService, DBusObjectPath, DBusInterface, "EventsAdded",
                       this, SLOT(onEventsAdded(QList<QVariantMap>)));
    connection.connect(DBusService, DBusObjectPath, DBusInterface, "EventsModified",
                       this, SLOT(onEventsAdded(QList<QVariantMap>)));
    connection.connect(DBusService, DBusObjectPath, DBusInterface, "EventsRemoved",
                       this, SLOT(onEventsRemoved(QList<QVariantMap>)));
}

ThreadPtr ManagerDBus::threadForParticipants(const QString &accountId,
                                             EventType type,
                                             const QStringList &participants,
                                             MatchFlags matchFlags,
                                             bool create)
{
    // FIXME: move to async call if possible
    QDBusReply<QVariantMap> reply = mInterface.call("ThreadForParticipants", accountId, (int) type, participants, (int)matchFlags, create);
    if (reply.isValid()) {
        QVariantMap properties = reply.value();
        return ItemFactory::instance()->createThread(properties);
    }

    return ThreadPtr();
}

bool ManagerDBus::writeEvents(const Events &events)
{
    // FIXME: implement
}

bool ManagerDBus::removeThreads(const Threads &threads)
{
    // FIXME: implement
}

bool ManagerDBus::removeEvents(const Events &events)
{
    // FIXME: implement
}

void ManagerDBus::onThreadsAdded(const QList<QVariantMap> &threads)
{
    Q_EMIT threadsAdded(threadsFromProperties(threads));
}

void ManagerDBus::onThreadsModified(const QList<QVariantMap> &threads)
{
    Q_EMIT threadsModified(threadsFromProperties(threads));
}

void ManagerDBus::onThreadsRemoved(const QList<QVariantMap> &threads)
{
    Q_EMIT threadsRemoved(threadsFromProperties(threads, true));
}

void ManagerDBus::onEventsAdded(const QList<QVariantMap> &events)
{
    Q_EMIT eventsAdded(eventsFromProperties(events));
}

void ManagerDBus::onEventsModified(const QList<QVariantMap> &events)
{
    Q_EMIT eventsModified(eventsFromProperties(events));
}

void ManagerDBus::onEventsRemoved(const QList<QVariantMap> &events)
{
    Q_EMIT eventsRemoved(eventsFromProperties(events));
}

Threads ManagerDBus::threadsFromProperties(const QList<QVariantMap> &threadsProperties, bool fakeIfNull)
{
    Threads threads;
    Q_FOREACH(const QVariantMap &map, threadsProperties) {
        ThreadPtr thread = ItemFactory::instance()->createThread(map);
        if (!thread.isNull()) {
            threads << thread;
        } else if (fakeIfNull) {
            threads << History::ItemFactory::instance()->createThread(map[FieldAccountId].toString(),
                                                                      map[FieldThreadId].toString(),
                                                                      (EventType)map[FieldType].toInt(),
                                                                      map[FieldParticipants].toStringList());
        }
    }

    return threads;
}

QList<QVariantMap> ManagerDBus::threadsToProperties(const Threads &threads)
{
    QList<QVariantMap> threadsPropertyMap;

    Q_FOREACH(const ThreadPtr &thread, threads) {
        threadsPropertyMap << thread->properties();
    }

    return threadsPropertyMap;
}

EventPtr ManagerDBus::eventFromProperties(const QVariantMap &properties)
{
    EventType type = (EventType)properties[FieldType].toInt();
    switch (type) {
    case EventTypeText:
        return ItemFactory::instance()->createTextEvent(properties);
    case EventTypeVoice:
        return ItemFactory::instance()->createVoiceEvent(properties);
    }
}

Events ManagerDBus::eventsFromProperties(const QList<QVariantMap> &eventsProperties)
{
    Events events;

    Q_FOREACH(const QVariantMap &map, eventsProperties) {
        events << eventFromProperties(map);
    }

    return events;
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

