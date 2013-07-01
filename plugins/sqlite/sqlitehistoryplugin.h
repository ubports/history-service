#ifndef SQLITEHISTORYPLUGIN_H
#define SQLITEHISTORYPLUGIN_H

#include <HistoryPlugin>
#include <QObject>

class SQLiteHistoryReader;
class SQLiteHistoryWriter;

typedef QSharedPointer<SQLiteHistoryReader> SQLiteHistoryReaderPtr;
typedef QSharedPointer<SQLiteHistoryWriter> SQLiteHistoryWriterPtr;

class SQLiteHistoryPlugin : public QObject, HistoryPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.canonical.libhistory.HistoryPlugin")
    Q_INTERFACES(HistoryPlugin)
public:
    explicit SQLiteHistoryPlugin(QObject *parent = 0);

    HistoryWriterPtr writer() const;
    HistoryReaderPtr reader() const;

private:
    SQLiteHistoryReaderPtr mReader;
    SQLiteHistoryWriterPtr mWriter;
};

#endif // SQLITEHISTORYPLUGIN_H
