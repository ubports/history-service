#ifndef SQLITEHISTORYTHREADVIEW_H
#define SQLITEHISTORYTHREADVIEW_H

#include "threadview.h"
#include "types.h"
#include <QSqlQuery>

class SQLiteHistoryReader;

class SQLiteHistoryThreadView : public History::ThreadView
{
public:
    SQLiteHistoryThreadView(SQLiteHistoryReader *reader,
                            History::EventType type,
                            const History::SortPtr &sort,
                            const History::FilterPtr &filter);

    QList<History::ThreadPtr> nextPage();
    bool isValid() const;

private:
    History::EventType mType;
    History::SortPtr mSort;
    History::FilterPtr mFilter;
    QSqlQuery mQuery;
    int mPageSize;
    SQLiteHistoryReader *mReader;
};

#endif // SQLITEHISTORYTHREADVIEW_H
