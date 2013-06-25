#include "sqlitehistoryplugin.h"
#include "sqlitehistorywriter.h"

SQLiteHistoryPlugin::SQLiteHistoryPlugin(QObject *parent) :
    QObject(parent), mWriter(new SQLiteHistoryWriter(this))
{
}

HistoryWriter *SQLiteHistoryPlugin::writer() const
{
    return mWriter;
}

HistoryReader *SQLiteHistoryPlugin::reader() const
{
    return 0;
}
