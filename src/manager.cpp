#include "manager.h"
#include "manager_p.h"
#include "pluginmanager.h"
#include "plugin.h"
#include "reader.h"
#include <QDebug>

namespace History
{

// ------------- ManagerPrivate ------------------------------------------------

ManagerPrivate::ManagerPrivate(const QString &theBackend)
{
}

ManagerPrivate::~ManagerPrivate()
{
}

// ------------- Manager -------------------------------------------------------

Manager::Manager(const QString &backendPlugin)
    : d_ptr(new ManagerPrivate(backendPlugin))
{
    Q_D(Manager);

    // try to find a plugin that has a reader
    Q_FOREACH(PluginPtr plugin, PluginManager::instance()->plugins()) {
        qDebug() << "Trying the plugin";
        d->reader = plugin->reader();
        if (d->reader) {
            break;
        }
    }
}

Manager::~Manager()
{
}

Manager *Manager::instance()
{
    static Manager *self = new Manager();
    return self;
}

ThreadViewPtr Manager::queryThreads(EventType type,
                                    const SortPtr &sort,
                                    const FilterPtr &filter)
{
    Q_D(Manager);
    if (d->reader) {
        return d->reader->queryThreads(type, sort, filter);
    }

    return ThreadViewPtr();
}

EventViewPtr Manager::queryEvents(EventType type,
                                  const SortPtr &sort,
                                  const FilterPtr &filter)
{
    Q_D(Manager);
    if (d->reader) {
        return d->reader->queryEvents(type, sort, filter);
    }

    return EventViewPtr();
}

bool Manager::removeThreads(History::EventType type, const QList<QString> &threadIds)
{
    Q_UNUSED(type)
    Q_UNUSED(threadIds)
    // FIXME: implement
}

bool Manager::removeEvents(History::EventType type, const QList<QString> &eventIds)
{
    Q_UNUSED(type)
    Q_UNUSED(eventIds)
    // FIXME: implement
}

}

