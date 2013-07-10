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
    ~HistoryManager();

    static HistoryManager *instance();

    HistoryThreadViewPtr queryThreads(HistoryItem::ItemType type,
                                      const HistorySortPtr &sort = HistorySortPtr(),
                                      const HistoryFilterPtr &filter = HistoryFilterPtr());

    HistoryItemViewPtr queryItems(HistoryItem::ItemType type,
                                     const HistorySortPtr &sort = HistorySortPtr(),
                                     const HistoryFilterPtr &filter = HistoryFilterPtr());

    bool removeThreads(HistoryItem::ItemType type, const QList<QString> &threadIds);
    bool removeItems(HistoryItem::ItemType type, const QList<QString> &itemIds);

private:
    HistoryManager(const QString &backendPlugin = QString::null);
    QScopedPointer<HistoryManagerPrivate> d_ptr;
};

#endif

