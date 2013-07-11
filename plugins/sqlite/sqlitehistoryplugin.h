#ifndef SQLITEHISTORYPLUGIN_H
#define SQLITEHISTORYPLUGIN_H

#include "plugin.h"
#include <QObject>

class SQLiteHistoryReader;
class SQLiteHistoryWriter;

typedef QSharedPointer<SQLiteHistoryReader> SQLiteHistoryReaderPtr;
typedef QSharedPointer<SQLiteHistoryWriter> SQLiteHistoryWriterPtr;

class SQLiteHistoryPlugin : public QObject, History::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.canonical.historyservice.Plugin")
    Q_INTERFACES(History::Plugin)
public:
    explicit SQLiteHistoryPlugin(QObject *parent = 0);

    History::WriterPtr writer() const;
    History::ReaderPtr reader() const;

private:
    SQLiteHistoryReaderPtr mReader;
    SQLiteHistoryWriterPtr mWriter;
};

#endif // SQLITEHISTORYPLUGIN_H
