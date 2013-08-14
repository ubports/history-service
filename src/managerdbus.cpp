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

static const char* DBUS_OBJECT_PATH = "/com/canonical/HistoryService";
static const char* HISTORY_INTERFACE = "com.canonical.HistoryService";

Q_DECLARE_METATYPE(QList< QVariantMap >)

namespace History
{

ManagerDBus::ManagerDBus(QObject *parent) :
    QObject(parent), mAdaptor(0)
{
    qDBusRegisterMetaType<QList<QVariantMap> >();
    connectToBus();

    // listen for signals coming from the bus
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.connect(QString::null, QString::null, HISTORY_INTERFACE, "ThreadsAdded",
                       this, SLOT(onThreadsAdded(QList<QVariantMap>)));
    connection.connect(QString::null, QString::null, HISTORY_INTERFACE, "ThreadsModified",
                       this, SLOT(onThreadsModified(QList<QVariantMap>)));
    connection.connect(QString::null, QString::null, HISTORY_INTERFACE, "ThreadsRemoved",
                       this, SLOT(onThreadsRemoved(QList<QVariantMap>)));

    connection.connect(QString::null, QString::null, HISTORY_INTERFACE, "EventsAdded",
                       this, SLOT(onEventsAdded(QList<QVariantMap>)));
    connection.connect(QString::null, QString::null, HISTORY_INTERFACE, "EventsModified",
                       this, SLOT(onEventsAdded(QList<QVariantMap>)));
    connection.connect(QString::null, QString::null, HISTORY_INTERFACE, "EventsRemoved",
                       this, SLOT(onEventsRemoved(QList<QVariantMap>)));
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
    Manager *manager = Manager::instance();

    Q_FOREACH(const QVariantMap &map, threadsProperties) {
        QString accountId = map["accountId"].toString();
        QString threadId = map["threadId"].toString();
        EventType type = (EventType) map["type"].toInt();
        ThreadPtr thread = manager->getSingleThread(type, accountId, threadId, false);
        if (!thread.isNull()) {
            threads << thread;
        } else if (fakeIfNull) {
            threads << History::ItemFactory::instance()->createThread(accountId, threadId, type, QStringList());
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

Events ManagerDBus::eventsFromProperties(const QList<QVariantMap> &eventsProperties)
{
    Events events;

    Q_FOREACH(const QVariantMap &map, eventsProperties) {
        EventPtr event;

        QString accountId = map["accountId"].toString();
        QString threadId = map["threadId"].toString();
        QString eventId = map["eventId"].toString();
        EventType type = (EventType) map["type"].toInt();
        QString senderId = map["senderId"].toString();
        QDateTime timestamp;
        map["timestamp"].value<QDBusArgument>() >> timestamp;
        bool newEvent = map["newEvent"].toBool();

        // now create the
        switch (type) {
        case EventTypeText: {
            QString message = map["message"].toString();
            MessageType messageType = (MessageType) map["messageType"].toInt();
            MessageFlags messageFlags = (MessageFlags) map["messageFlags"].toInt();
            QDateTime readTimestamp;
            map["readTimestamp"].value<QDBusArgument>() >> readTimestamp;
            event = History::ItemFactory::instance()->createTextEvent(accountId, threadId, eventId, senderId, timestamp, newEvent,
                                                                      message, messageType, messageFlags, readTimestamp);
            break;
        }
        case EventTypeVoice: {
            bool missed = map["missed"].toBool();
            QTime duration = QTime(0,0,0).addSecs(map["duration"].toInt());
            event = History::ItemFactory::instance()->createVoiceEvent(accountId, threadId, eventId, senderId, timestamp, newEvent,
                                                                       missed, duration);
            break;
        }
        }

        events << event;
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

