#include "historymanager.h"
#include "historymanager_p.h"

// ------------- HistoryManagerPrivate ------------------------------------------------

HistoryManagerPrivate::HistoryManagerPrivate(const QString &theBackend)
{
}

HistoryManagerPrivate::~HistoryManagerPrivate()
{
}

// ------------- HistoryManager -------------------------------------------------------

HistoryManager::HistoryManager(const QString &backendPlugin)
    : d_ptr(new HistoryManagerPrivate(backendPlugin))
{
}

HistoryManager::~HistoryManager()
{
}

QList<HistoryThreadPtr> HistoryManager::queryThreads(HistoryItem::ItemType type,
                                                     const HistorySort &sort,
                                                     const HistoryFilter &filter,
                                                     int startOffset,
                                                     int pageSize)
{
    return QList<HistoryThreadPtr>();
}

QList<HistoryItemPtr> HistoryManager::queryItems(HistoryItem::ItemType type,
                                                 const HistorySort &sort,
                                                 const HistoryFilter &filter,
                                                 int startOffset,
                                                 int pageSize)
{
    return QList<HistoryItemPtr>();
}

