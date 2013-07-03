#ifndef SQLITEHISTORYREADER_H
#define SQLITEHISTORYREADER_H

#include <HistoryReader>

class SQLiteHistoryReader : public HistoryReader
{
    Q_OBJECT
public:
    explicit SQLiteHistoryReader(QObject *parent = 0);
    QList<HistoryThreadPtr> queryThreads(HistoryItem::ItemType type,
                                         const HistorySort &sort = HistorySort(),
                                         const HistoryFilter &filter = HistoryFilter(),
                                         int startOffset = 0,
                                         int pageSize = -1);
    QList<HistoryItemPtr> queryItems(HistoryItem::ItemType type,
                                     const HistorySort &sort = HistorySort(),
                                     const HistoryFilter &filter = HistoryFilter(),
                                     int startOffset = 0,
                                     int pageSize = -1);
protected:
    QList<HistoryItemPtr> queryTextItems(const HistorySort &sort = HistorySort(),
                                         const HistoryFilter &filter = HistoryFilter(),
                                         int startOffset = 0,
                                         int pageSize = -1);
    QList<HistoryItemPtr> queryVoiceItems(const HistorySort &sort = HistorySort(),
                                          const HistoryFilter &filter = HistoryFilter(),
                                          int startOffset = 0,
                                          int pageSize = -1);

private:
    QString pageSqlCommand(int startOffset, int pageSize) const;

};

#endif // SQLITEHISTORYREADER_H
