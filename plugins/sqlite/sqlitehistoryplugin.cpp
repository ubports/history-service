#include "sqlitehistoryplugin.h"
#include "sqlitehistoryreader.h"
#include "sqlitehistorywriter.h"
#include <QDebug>

SQLiteHistoryPlugin::SQLiteHistoryPlugin(QObject *parent) :
    QObject(parent), mReader(new SQLiteHistoryReader()), mWriter(new SQLiteHistoryWriter())
{
}

History::WriterPtr SQLiteHistoryPlugin::writer() const
{
    return mWriter;
}

History::ReaderPtr SQLiteHistoryPlugin::reader() const
{
    return mReader;
}
