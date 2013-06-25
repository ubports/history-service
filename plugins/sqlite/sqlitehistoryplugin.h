#ifndef SQLITEHISTORYPLUGIN_H
#define SQLITEHISTORYPLUGIN_H

#include <HistoryPlugin>
#include <QObject>

class SQLiteHistoryWriter;

class SQLiteHistoryPlugin : public QObject, HistoryPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.canonical.libhistory.HistoryPlugin")
    Q_INTERFACES(HistoryPlugin)
public:
    explicit SQLiteHistoryPlugin(QObject *parent = 0);

    HistoryWriter *writer() const;
    HistoryReader *reader() const;

private:
    SQLiteHistoryWriter *mWriter;
};

#endif // SQLITEHISTORYPLUGIN_H
