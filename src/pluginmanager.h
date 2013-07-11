#ifndef HISTORY_PLUGINMANAGER_H
#define HISTORY_PLUGINMANAGER_H

#include <QObject>
#include "types.h"

namespace History
{

class Plugin;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    ~PluginManager();
    static PluginManager *instance();
    QList<PluginPtr> plugins();

protected:
    void loadPlugins();

private:
    explicit PluginManager(QObject *parent = 0);
    QList<PluginPtr> mPlugins;
};

}

#endif // HISTORY_PLUGINMANAGER_H
