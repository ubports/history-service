#include "sqlitehistoryplugin.h"
#include "sqlitehistoryreader.h"
#include "sqlitehistorywriter.h"
#include <QDebug>

SQLiteHistoryPlugin::SQLiteHistoryPlugin(QObject *parent) :
    QObject(parent), mReader(new SQLiteHistoryReader()), mWriter(new SQLiteHistoryWriter())
{
}

HistoryWriterPtr SQLiteHistoryPlugin::writer() const
{
    return mWriter;
}

HistoryReaderPtr SQLiteHistoryPlugin::reader() const
{
    return mReader;
}
