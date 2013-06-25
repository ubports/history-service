#include "sqlitehistoryplugin.h"
#include "sqlitehistorywriter.h"

SQLiteHistoryPlugin::SQLiteHistoryPlugin(QObject *parent) :
    QObject(parent), mWriter(new SQLiteHistoryWriter(this))
{
}

HistoryWriterPtr SQLiteHistoryPlugin::writer() const
{
    return mWriter;
}

HistoryReaderPtr SQLiteHistoryPlugin::reader() const
{
    return HistoryReaderPtr();
}
