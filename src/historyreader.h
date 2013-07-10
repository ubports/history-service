#ifndef HISTORYREADER_H
#define HISTORYREADER_H

#include <QObject>
#include <Types>
#include <HistoryItem>
#include <HistoryFilter>
#include <HistorySort>

class HistoryReader : public QObject
{
    Q_OBJECT
public:
    explicit HistoryReader(QObject *parent = 0) : QObject(parent) {}
    virtual ~HistoryReader() {}

    virtual HistoryThreadViewPtr queryThreads(HistoryItem::ItemType type,
                                              const HistorySortPtr &sort = HistorySortPtr(),
                                              const HistoryFilterPtr &filter = HistoryFilterPtr()) = 0;
    virtual HistoryItemViewPtr queryItems(HistoryItem::ItemType type,
                                          const HistorySortPtr &sort = HistorySortPtr(),
                                          const HistoryFilterPtr &filter = HistoryFilterPtr()) = 0;
};

#endif
