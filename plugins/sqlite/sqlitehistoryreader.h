#ifndef SQLITEHISTORYREADER_H
#define SQLITEHISTORYREADER_H

#include <HistoryReader>

class SQLiteHistoryReader : public HistoryReader
{
    Q_OBJECT
public:
    explicit SQLiteHistoryReader(QObject *parent = 0);
    HistoryThreadViewPtr queryThreads(HistoryItem::ItemType type,
                                      const HistorySort &sort = HistorySort(),
                                      const HistoryFilter &filter = HistoryFilter());
    HistoryItemViewPtr queryItems(HistoryItem::ItemType type,
                                  const HistorySort &sort = HistorySort(),
                                  const HistoryFilter &filter = HistoryFilter());
};

#endif // SQLITEHISTORYREADER_H
