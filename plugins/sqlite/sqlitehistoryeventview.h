#ifndef SQLITEHISTORYEVENTVIEW_H
#define SQLITEHISTORYEVENTVIEW_H

#include "eventview.h"
#include "event.h"
#include "types.h"
#include <QSqlQuery>

class SQLiteHistoryReader;

class SQLiteHistoryEventView : public History::EventView
{
public:
    SQLiteHistoryEventView(SQLiteHistoryReader *reader,
                          History::EventType type,
                          const History::SortPtr &sort,
                          const History::FilterPtr &filter);

    QList<History::EventPtr> nextPage();
    bool isValid() const;

protected:


private:
    History::EventType mType;
    History::SortPtr mSort;
    History::FilterPtr mFilter;
    QSqlQuery mQuery;
    int mPageSize;
    SQLiteHistoryReader *mReader;
};

#endif // SQLITEHISTORYEVENTVIEW_H
