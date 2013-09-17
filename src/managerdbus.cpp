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
#include "event.h"
#include "manager.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDBusReply>
#include <QDBusMetaType>

Q_DECLARE_METATYPE(QList< QVariantMap >)

namespace History
{

ManagerDBus::ManagerDBus(QObject *parent) :
    QObject(parent), mAdaptor(0), mInterface(DBusService,
                                             DBusObjectPath,
                                             DBusInterface)
{
    qDBusRegisterMetaType<QList<QVariantMap> >();
    qRegisterMetaType<QList<QVariantMap> >();

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
                       this, SLOT(onEventsModified(QList<QVariantMap>)));
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
        return Thread::fromProperties(properties);
    }

    return ThreadPtr();
}

bool ManagerDBus::writeEvents(const Events &events)
{
    QList<QVariantMap> eventMap = eventsToProperties(events);
    if (eventMap.isEmpty()) {
        return false;
    }

    QDBusReply<bool> reply = mInterface.call("WriteEvents", QVariant::fromValue(eventMap));
    if (!reply.isValid()) {
        return false;
    }
    return reply.value();
}

bool ManagerDBus::removeThreads(const Threads &threads)
{
    QList<QVariantMap> threadMap = threadsToProperties(threads);
    if (threadMap.isEmpty()) {
        return false;
    }

    QDBusReply<bool> reply = mInterface.call("RemoveThreads", QVariant::fromValue(threadMap));
    if (!reply.isValid()) {
        return false;
    }
    return reply.value();
}

bool ManagerDBus::removeEvents(const Events &events)
{
    QList<QVariantMap> eventMap = eventsToProperties(events);
    if (eventMap.isEmpty()) {
        return false;
    }

    QDBusReply<bool> reply = mInterface.call("RemoveEvents", QVariant::fromValue(eventMap));
    if (!reply.isValid()) {
        return false;
    }
    return reply.value();
}

ThreadPtr ManagerDBus::getSingleThread(EventType type, const QString &accountId, const QString &threadId)
{
    ThreadPtr thread;
    QDBusReply<QVariantMap> reply = mInterface.call("GetSingleThread", (int)type, accountId, threadId);
    if (!reply.isValid()) {
        return thread;
    }

    thread = Thread::fromProperties(reply.value());
    return thread;
}

Event ManagerDBus::getSingleEvent(EventType type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    Event event;
    QDBusReply<QVariantMap> reply = mInterface.call("GetSingleEvent", (int)type, accountId, threadId, eventId);
    if (!reply.isValid()) {
        return event;
    }

    event = eventFromProperties(reply.value());
    return event;
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
        ThreadPtr thread = Thread::fromProperties(map);
        if (!thread.isNull()) {
            threads << thread;
        } else if (fakeIfNull) {
            threads << ThreadPtr(new Thread(map[FieldAccountId].toString(),
                                            map[FieldThreadId].toString(),
                                            (EventType)map[FieldType].toInt(),
                                            map[FieldParticipants].toStringList()));
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

Event ManagerDBus::eventFromProperties(const QVariantMap &properties)
{
    EventType type = (EventType)properties[FieldType].toInt();
    switch (type) {
    case EventTypeText:
        return TextEvent::fromProperties(properties);
    case EventTypeVoice:
        return VoiceEvent::fromProperties(properties);
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

    Q_FOREACH(const Event &event, events) {
        eventsPropertyMap << event.properties();
    }

    return eventsPropertyMap;
}


}

