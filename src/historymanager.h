#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QObject>
#include <QString>
#include <Types>
#include <HistoryFilter>
#include <HistoryItem>
#include <HistorySort>

class HistoryManagerPrivate;

class HistoryManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(HistoryManager)

public:
    HistoryManager(const QString &backendPlugin = QString::null);
    ~HistoryManager();

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

    bool removeThreads(HistoryItem::ItemType type, const QList<QString> &threadIds);
    bool removeItems(HistoryItem::ItemType type, const QList<QString> &itemIds);

protected:
    QScopedPointer<HistoryManagerPrivate> d_ptr;
};

#endif

