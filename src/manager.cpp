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

#include "manager.h"
#include "manager_p.h"
#include "managerdbus_p.h"
#include "pluginmanager_p.h"
#include "plugin.h"
#include "textevent.h"
#include "voiceevent.h"
#include "reader.h"
#include "writer.h"
#include <QDebug>

#define HISTORY_INTERFACE "com.canonical.HistoryService"

namespace History
{

// ------------- ManagerPrivate ------------------------------------------------

ManagerPrivate::ManagerPrivate(const QString &theBackend)
    : dbus(new ManagerDBus())
{
}

ManagerPrivate::~ManagerPrivate()
{
}

// ------------- Manager -------------------------------------------------------

Manager::Manager(const QString &backendPlugin)
    : d_ptr(new ManagerPrivate(backendPlugin))
{
    Q_D(Manager);

    // FIXME: maybe we should look for both at once
    // try to find a plugin that has a reader
    Q_FOREACH(PluginPtr plugin, PluginManager::instance()->plugins()) {
        d->reader = plugin->reader();
        if (d->reader) {
            break;
        }
    }

    // and now a plugin that has a writer
    Q_FOREACH(PluginPtr plugin, PluginManager::instance()->plugins()) {
        d->writer = plugin->writer();
        if (d->writer) {
            break;
        }
    }
    // FIXME: connect signals
}

Manager::~Manager()
{
}

Manager *Manager::instance()
{
    static Manager *self = new Manager();
    return self;
}

ThreadViewPtr Manager::queryThreads(EventType type,
                                    const SortPtr &sort,
                                    const FilterPtr &filter)
{
    Q_D(Manager);
    if (d->reader) {
        return d->reader->queryThreads(type, sort, filter);
    }

    return ThreadViewPtr();
}

EventViewPtr Manager::queryEvents(EventType type,
                                  const SortPtr &sort,
                                  const FilterPtr &filter)
{
    Q_D(Manager);
    if (d->reader) {
        return d->reader->queryEvents(type, sort, filter);
    }

    return EventViewPtr();
}

ThreadPtr Manager::threadForParticipants(const QString &accountId,
                                         EventType type,
                                         const QStringList &participants,
                                         bool create)
{
    Q_D(Manager);

    if (!d->reader || !d->writer) {
        return ThreadPtr();
    }

    ThreadPtr thread = d->reader->threadForParticipants(accountId, type, participants);

    // if the thread is null, create a new if possible/desired.
    if (thread.isNull() && create) {
        thread = d->writer->createThreadForParticipants(accountId, type, participants);
        d->dbus->notifyThreadsAdded(Threads() << thread);
    }

    return thread;
}

bool Manager::writeTextEvents(const TextEvents &textEvents)
{
    Q_D(Manager);

    if (!d->writer) {
        return false;
    }

    d->writer->beginBatchOperation();

    Events events;
    Q_FOREACH(const TextEventPtr &textEvent, textEvents) {
        d->writer->writeTextEvent(textEvent);
        events << textEvent.staticCast<Event>();
    }

    d->writer->endBatchOperation();

    d->dbus->notifyEventsAdded(events);
    return true;
}

bool Manager::writeVoiceEvents(const VoiceEvents &voiceEvents)
{
    Q_D(Manager);

    if (!d->writer) {
        return false;
    }

    d->writer->beginBatchOperation();

    Events events;
    Q_FOREACH(const VoiceEventPtr &voiceEvent, voiceEvents) {
        d->writer->writeVoiceEvent(voiceEvent);
        events << voiceEvent.staticCast<Event>();
    }

    d->writer->endBatchOperation();

    d->dbus->notifyEventsAdded(events);
    return true;
}

bool Manager::removeThreads(History::EventType type, const Threads &threads)
{
    Q_UNUSED(type)
    Q_UNUSED(threads)
    // FIXME: implement
}

bool Manager::removeEvents(History::EventType type, const Events &events)
{
    Q_UNUSED(type)
    Q_UNUSED(events)
    // FIXME: implement
}

}

