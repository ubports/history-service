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

#include "pluginmanager.h"
#include "config.h"
#include "plugin.h"
#include <QDir>
#include <QPluginLoader>
#include <QDebug>

namespace History
{

PluginManager::PluginManager(QObject *parent) :
    QObject(parent)
{
    loadPlugins();
}

PluginManager::~PluginManager()
{
}

PluginManager *PluginManager::instance()
{
    static PluginManager *self = new PluginManager();
    return self;
}

Plugins PluginManager::plugins()
{
    return mPlugins;
}


void PluginManager::loadPlugins()
{
    QDir dir(HISTORY_PLUGIN_PATH);

    Q_FOREACH (QString fileName, dir.entryList(QDir::Files)) {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        Plugin *plugin = qobject_cast<Plugin*>(loader.instance());
        if (plugin) {
            mPlugins.append(PluginPtr(plugin));
        }
    }
}

}
