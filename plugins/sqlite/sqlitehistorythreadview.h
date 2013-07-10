#ifndef SQLITEHISTORYTHREADVIEW_H
#define SQLITEHISTORYTHREADVIEW_H

#include <HistoryThreadView>
#include <HistoryItem>
#include <HistorySort>
#include <HistoryFilter>
#include <QSqlQuery>

class SQLiteHistoryReader;

class SQLiteHistoryThreadView : public HistoryThreadView
{
public:
    SQLiteHistoryThreadView(SQLiteHistoryReader *reader,
                            HistoryItem::ItemType type,
                            const HistorySortPtr &sort,
                            const HistoryFilterPtr &filter);

    QList<HistoryThreadPtr> nextPage();
    bool isValid() const;

private:
    HistoryItem::ItemType mType;
    HistorySortPtr mSort;
    HistoryFilterPtr mFilter;
    QSqlQuery mQuery;
    int mPageSize;
    SQLiteHistoryReader *mReader;
};

#endif // SQLITEHISTORYTHREADVIEW_H
