#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>

class HistoryPlugin;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    ~PluginManager();
    static PluginManager *instance();
    QList<HistoryPlugin*> plugins();

protected:
    void loadPlugins();

private:
    explicit PluginManager(QObject *parent = 0);
    QList<HistoryPlugin*> mPlugins;
};

#endif // PLUGINMANAGER_H
