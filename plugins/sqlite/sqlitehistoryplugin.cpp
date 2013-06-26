#include "sqlitehistoryplugin.h"
#include "sqlitehistorywriter.h"
#include <QDebug>

SQLiteHistoryPlugin::SQLiteHistoryPlugin(QObject *parent) :
    QObject(parent), mWriter(new SQLiteHistoryWriter())
{
}

HistoryWriterPtr SQLiteHistoryPlugin::writer() const
{
    qDebug() << __PRETTY_FUNCTION__;
    return mWriter;
}

HistoryReaderPtr SQLiteHistoryPlugin::reader() const
{
    return HistoryReaderPtr();
}
