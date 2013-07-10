#include "sqlitehistoryreader.h"
#include "sqlitehistoryitemview.h"
#include "sqlitehistorythreadview.h"
#include "sqlitedatabase.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <HistoryFilter>
#include <HistoryIntersectionFilter>
#include <HistoryThread>
#include <TextItem>
#include <VoiceItem>

SQLiteHistoryReader::SQLiteHistoryReader(QObject *parent) :
    HistoryReader(parent)
{
}

HistoryThreadViewPtr SQLiteHistoryReader::queryThreads(HistoryItem::ItemType type,
                                                       const HistorySortPtr &sort,
                                                       const HistoryFilterPtr &filter)
{
    return HistoryThreadViewPtr(new SQLiteHistoryThreadView(this, type, sort, filter));
}

HistoryItemViewPtr SQLiteHistoryReader::queryItems(HistoryItem::ItemType type,
                                                      const HistorySortPtr &sort,
                                                      const HistoryFilterPtr &filter)
{
    return HistoryItemViewPtr(new SQLiteHistoryItemView(this, type, sort, filter));
}
