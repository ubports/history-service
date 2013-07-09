#include "historymanager.h"
#include "historymanager_p.h"
#include <PluginManager>
#include <HistoryPlugin>
#include <HistoryReader>
#include <QDebug>

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
    Q_D(HistoryManager);

    // try to find a plugin that has a reader
    Q_FOREACH(HistoryPlugin *plugin, PluginManager::instance()->plugins()) {
        qDebug() << "Trying the plugin";
        d->reader = plugin->reader();
        if (d->reader) {
            break;
        }
    }
}

HistoryManager::~HistoryManager()
{
}

HistoryManager *HistoryManager::instance()
{
    static HistoryManager *self = new HistoryManager();
    return self;
}

HistoryThreadViewPtr HistoryManager::queryThreads(HistoryItem::ItemType type,
                                                  const HistorySort &sort,
                                                  const HistoryFilter &filter)
{
    Q_D(HistoryManager);
    if (d->reader) {
        return d->reader->queryThreads(type, sort, filter);
    }

    return HistoryThreadViewPtr();
}

QList<HistoryItemPtr> HistoryManager::queryItems(HistoryItem::ItemType type,
                                                 const HistorySort &sort,
                                                 const HistoryFilter &filter,
                                                 int startOffset,
                                                 int pageSize)
{
    Q_D(HistoryManager);
    if (d->reader) {
        return d->reader->queryItems(type, sort, filter, startOffset, pageSize);
    }

    return QList<HistoryItemPtr>();
}

