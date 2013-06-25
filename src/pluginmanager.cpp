#include "pluginmanager.h"
#include "config.h"
#include <QDir>
#include <QPluginLoader>
#include <HistoryPlugin>
#include <QDebug>

PluginManager::PluginManager(QObject *parent) :
    QObject(parent)
{
    loadPlugins();
}

PluginManager::~PluginManager()
{
    // remove all the loaded plugins
    qDeleteAll(mPlugins);
}

PluginManager *PluginManager::instance()
{
    static PluginManager *self = new PluginManager();
    return self;
}

QList<HistoryPlugin *> PluginManager::plugins()
{
    return mPlugins;
}


void PluginManager::loadPlugins()
{
    QDir dir(HISTORY_PLUGIN_PATH);

    Q_FOREACH (QString fileName, dir.entryList(QDir::Files)) {
        qDebug() << fileName;
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        HistoryPlugin *plugin = qobject_cast<HistoryPlugin*>(loader.instance());
        if (plugin) {
            mPlugins.append(plugin);
        }
    }
}
