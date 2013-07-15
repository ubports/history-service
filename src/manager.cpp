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
#include "pluginmanager.h"
#include "plugin.h"
#include "reader.h"
#include <QDebug>

namespace History
{

// ------------- ManagerPrivate ------------------------------------------------

ManagerPrivate::ManagerPrivate(const QString &theBackend)
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

    // try to find a plugin that has a reader
    Q_FOREACH(PluginPtr plugin, PluginManager::instance()->plugins()) {
        qDebug() << "Trying the plugin";
        d->reader = plugin->reader();
        if (d->reader) {
            break;
        }
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

bool Manager::removeThreads(History::EventType type, const QList<QString> &threadIds)
{
    Q_UNUSED(type)
    Q_UNUSED(threadIds)
    // FIXME: implement
}

bool Manager::removeEvents(History::EventType type, const QList<QString> &eventIds)
{
    Q_UNUSED(type)
    Q_UNUSED(eventIds)
    // FIXME: implement
}

}

