#ifndef SQLITEHISTORYREADER_H
#define SQLITEHISTORYREADER_H

#include <HistoryReader>

class SQLiteHistoryReader : public HistoryReader
{
    Q_OBJECT
public:
    explicit SQLiteHistoryReader(QObject *parent = 0);
    HistoryThreadViewPtr queryThreads(HistoryItem::ItemType type,
                                      const HistorySortPtr &sort = HistorySortPtr(),
                                      const HistoryFilterPtr &filter = HistoryFilterPtr());
    HistoryItemViewPtr queryItems(HistoryItem::ItemType type,
                                  const HistorySortPtr &sort = HistorySortPtr(),
                                  const HistoryFilterPtr &filter = HistoryFilterPtr());
};

#endif // SQLITEHISTORYREADER_H
