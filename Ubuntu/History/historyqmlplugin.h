#ifndef HISTORYQMLPLUGIN_H
#define HISTORYQMLPLUGIN_H

#include <QQmlExtensionPlugin>

class HistoryQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void initializeEngine(QQmlEngine *engine, const char *uri);
    void registerTypes(const char *uri);
};

#endif // HISTORYQMLPLUGIN_H
