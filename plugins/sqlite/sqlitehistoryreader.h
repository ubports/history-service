#ifndef SQLITEHISTORYREADER_H
#define SQLITEHISTORYREADER_H

#include "reader.h"

class SQLiteHistoryReader : public History::Reader
{
    Q_OBJECT
public:
    explicit SQLiteHistoryReader(QObject *parent = 0);
    History::ThreadViewPtr queryThreads(History::EventType type,
                                        const History::SortPtr &sort = History::SortPtr(),
                                        const History::FilterPtr &filter = History::FilterPtr());
    History::EventViewPtr queryEvents(History::EventType type,
                                      const History::SortPtr &sort = History::SortPtr(),
                                      const History::FilterPtr &filter = History::FilterPtr());
};

#endif // SQLITEHISTORYREADER_H
