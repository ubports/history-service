#ifndef SQLITEHISTORYITEMVIEW_H
#define SQLITEHISTORYITEMVIEW_H

#include <HistoryItemView>
#include <HistoryItem>
#include <HistorySort>
#include <HistoryFilter>
#include <QSqlQuery>

class SQLiteHistoryReader;

class SQLiteHistoryItemView : public HistoryItemView
{
public:
    SQLiteHistoryItemView(SQLiteHistoryReader *reader,
                          HistoryItem::ItemType type,
                          const HistorySortPtr &sort,
                          const HistoryFilterPtr &filter);

    QList<HistoryItemPtr> nextPage();
    bool isValid() const;

protected:


private:
    HistoryItem::ItemType mType;
    HistorySortPtr mSort;
    HistoryFilterPtr mFilter;
    QSqlQuery mQuery;
    int mPageSize;
    SQLiteHistoryReader *mReader;
};

#endif // SQLITEHISTORYITEMVIEW_H
