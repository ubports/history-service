/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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
#include "textevent.h"
#include "thread.h"
#include "threadview.h"
#include "voiceevent.h"
#include <QDebug>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>

namespace History
{

// ------------- ManagerPrivate ------------------------------------------------

ManagerPrivate::ManagerPrivate()
    : dbus(new ManagerDBus()), serviceWatcher(DBusService, QDBusConnection::sessionBus())
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
            SIGNAL(threadParticipantsChanged(History::Thread, History::Participants, History::Participants, History::Participants)),
            SIGNAL(threadParticipantsChanged(History::Thread, History::Participants, History::Participants, History::Participants)));
    connect(d->dbus.data(),
            SIGNAL(eventsAdded(History::Events)),
            SIGNAL(eventsAdded(History::Events)));
    connect(d->dbus.data(),
            SIGNAL(eventsModified(History::Events)),
            SIGNAL(eventsModified(History::Events)));
    connect(d->dbus.data(),
            SIGNAL(eventsRemoved(History::Events)),
            SIGNAL(eventsRemoved(History::Events)));

    // watch for the service going up and down
    connect(&d->serviceWatcher, &QDBusServiceWatcher::serviceRegistered, [&](const QString &serviceName) {
        qDebug() << "HistoryService: service registered:" << serviceName;
        this->d_ptr->serviceRunning = true;
        Q_EMIT this->serviceRunningChanged();
    });
    connect(&d->serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, [&](const QString &serviceName) {
        qDebug() << "HistoryService: service unregistered:" << serviceName;
        this->d_ptr->serviceRunning = false;
        Q_EMIT this->serviceRunningChanged();
    });

    // and fetch the current status
    d->serviceRunning = false;
    QDBusReply<bool> reply = QDBusConnection::sessionBus().interface()->isServiceRegistered(DBusService);
    if (reply.isValid()) {
        d->serviceRunning = reply.value();
    }
}

Manager::~Manager()
{
}

Manager *Manager::instance()
{
    static Manager *self = new Manager();
    return self;
}

void Manager::markThreadsAsRead(const History::Threads &threads)
{
    Q_D(Manager);

    d->dbus->markThreadsAsRead(threads);
}

ThreadViewPtr Manager::queryThreads(EventType type,
                                    const Sort &sort,
                                    const Filter &filter,
                                    const QVariantMap &properties)
{
    return ThreadViewPtr(new ThreadView(type, sort, filter, properties));
}

EventViewPtr Manager::queryEvents(EventType type,
                                  const Sort &sort,
                                  const Filter &filter)
{
    return EventViewPtr(new EventView(type, sort, filter));
}

Event Manager::getSingleEvent(EventType type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    Q_D(Manager);

    Event event = d->dbus->getSingleEvent(type, accountId, threadId, eventId);
    return event;
}

Thread Manager::threadForParticipants(const QString &accountId,
                                         EventType type,
                                         const QStringList &participants,
                                         MatchFlags matchFlags,
                                         bool create)
{
    Q_D(Manager);

    QVariantMap properties;
    properties[History::FieldParticipantIds] = participants;
    if (participants.size() == 1) {
        properties[History::FieldChatType] = History::ChatTypeContact;
    }
    return d->dbus->threadForProperties(accountId, type, properties, matchFlags, create);
}

Thread Manager::threadForProperties(const QString &accountId,
                                    EventType type,
                                    const QVariantMap &properties,
                                    MatchFlags matchFlags,
                                    bool create)
{
    Q_D(Manager);

    return d->dbus->threadForProperties(accountId, type, properties, matchFlags, create);
}

/**
 * @brief Request the list of participants of the given threads to the service
 * @param threads The threads to be filled
 *
 * This is an asychronous request. When finished, the signal @ref threadParticipantsChanged
 * will be emitted for the given threads.
 */
void Manager::requestThreadParticipants(const Threads &threads)
{
    Q_D(Manager);

    d->dbus->requestThreadParticipants(threads);
}

Thread Manager::getSingleThread(EventType type, const QString &accountId, const QString &threadId, const QVariantMap &properties)
{
    Q_D(Manager);

    Thread thread = d->dbus->getSingleThread(type, accountId, threadId, properties);
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

bool Manager::removeEvents(EventType type, const Filter &filter, const Sort &sort)
{
    Q_D(Manager);
    return d->dbus->removeEvents(type, filter, sort);
}

int Manager::eventsCount(EventType type, const Filter &filter)
{
    Q_D(Manager);
    return d->dbus->eventsCount(type, filter);
}

bool Manager::isServiceRunning() const
{
    Q_D(const Manager);
    return d->serviceRunning;
}

}

