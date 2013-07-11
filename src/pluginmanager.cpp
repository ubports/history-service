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

QList<PluginPtr> PluginManager::plugins()
{
    return mPlugins;
}


void PluginManager::loadPlugins()
{
    QDir dir(HISTORY_PLUGIN_PATH);

    Q_FOREACH (QString fileName, dir.entryList(QDir::Files)) {
        qDebug() << fileName;
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        Plugin *plugin = qobject_cast<Plugin*>(loader.instance());
        if (plugin) {
            mPlugins.append(PluginPtr(plugin));
        }
    }
}

}
