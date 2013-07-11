#include "sqlitehistoryreader.h"
#include "sqlitehistoryeventview.h"
#include "sqlitehistorythreadview.h"
#include "sqlitedatabase.h"
#include "types.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

SQLiteHistoryReader::SQLiteHistoryReader(QObject *parent) :
    History::Reader(parent)
{
}

History::ThreadViewPtr SQLiteHistoryReader::queryThreads(History::EventType type,
                                                         const History::SortPtr &sort,
                                                         const History::FilterPtr &filter)
{
    return History::ThreadViewPtr(new SQLiteHistoryThreadView(this, type, sort, filter));
}

History::EventViewPtr SQLiteHistoryReader::queryEvents(History::EventType type,
                                                       const History::SortPtr &sort,
                                                       const History::FilterPtr &filter)
{
    return History::EventViewPtr(new SQLiteHistoryEventView(this, type, sort, filter));
}
