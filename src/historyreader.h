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

    virtual QList<HistoryThreadPtr> queryThreads(HistoryItem::ItemType type,
                                                 const HistorySort &sort = HistorySort(),
                                                 const HistoryFilter &filter = HistoryFilter(),
                                                 int startOffset = 0,
                                                 int pageSize = -1) = 0;
    virtual QList<HistoryItemPtr> queryItems(HistoryItem::ItemType type,
                                             const HistorySort &sort = HistorySort(),
                                             const HistoryFilter &filter = HistoryFilter(),
                                             int startOffset = 0,
                                             int pageSize = -1) = 0;
};

#endif
