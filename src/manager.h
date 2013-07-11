#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <QObject>
#include <QString>
#include "types.h"

namespace History
{

class ManagerPrivate;

class Manager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Manager)

public:
    ~Manager();

    static Manager *instance();

    ThreadViewPtr queryThreads(EventType type,
                               const SortPtr &sort = SortPtr(),
                               const FilterPtr &filter = FilterPtr());

    EventViewPtr queryEvents(EventType type,
                             const SortPtr &sort = SortPtr(),
                             const FilterPtr &filter = FilterPtr());

    bool removeThreads(EventType type, const QList<QString> &threadIds);
    bool removeEvents(EventType type, const QList<QString> &eventIds);

private:
    Manager(const QString &backendPlugin = QString::null);
    QScopedPointer<ManagerPrivate> d_ptr;
};

}

#endif
