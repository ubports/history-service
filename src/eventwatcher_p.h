/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EVENTWATCHER_P_H
#define EVENTWATCHER_P_H

#include "types.h"
#include <QObject>
#include <QList>
#include <QVariantMap>

namespace History
{

class EventWatcher : public QObject
{
    Q_OBJECT
public:
    static EventWatcher *instance();

Q_SIGNALS:
    void threadsAdded(const History::Threads &threads);
    void threadsModified(const History::Threads &threads);
    void threadsRemoved(const History::Threads &threads);

    void eventsAdded(const History::Events &events);
    void eventsModified(const History::Events &events);
    void eventsRemoved(const History::Events &events);

protected Q_SLOTS:
    void onThreadsAdded(const QList<QVariantMap> &threads);
    void onThreadsModified(const QList<QVariantMap> &threads);
    void onThreadsRemoved(const QList<QVariantMap> &threads);

    void onEventsAdded(const QList<QVariantMap> &events);
    void onEventsModified(const QList<QVariantMap> &events);
    void onEventsRemoved(const QList<QVariantMap> &events);

protected:
    Threads threadsFromProperties(const QList<QVariantMap> &threadsProperties);
    Events eventsFromProperties(const QList<QVariantMap> &eventsProperties);

private:
    explicit EventWatcher(QObject *parent = 0);
};

}

#endif // EVENTWATCHER_P_H
