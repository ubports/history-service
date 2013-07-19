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

#ifndef MANAGERDBUS_P_H
#define MANAGERDBUS_P_H

#include <QDBusContext>
#include <QObject>
#include "types.h"

class HistoryServiceAdaptor;

namespace History
{

class ManagerDBus : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit ManagerDBus(QObject *parent = 0);

    bool connectToBus();

    void notifyThreadsAdded(const Threads &threads);
    void notifyThreadsModified(const Threads &threads);
    void notifyThreadsRemoved(const Threads &threads);

    void notifyEventsAdded(const Events &events);
    void notifyEventsModified(const Events &events);
    void notifyEventsRemoved(const Events &events);

Q_SIGNALS:
    void ThreadsAdded(const QList<QVariantMap> &threads);
    void ThreadsModified(const QList<QVariantMap> &threads);
    void ThreadsRemoved(const QList<QVariantMap> &threads);

    void EventsAdded(const QList<QVariantMap> &events);
    void EventsModified(const QList<QVariantMap> &events);
    void EventsRemoved(const QList<QVariantMap> &events);

protected:
    QList<QVariantMap> threadsToProperties(const Threads &threads);
    QList<QVariantMap> eventsToProperties(const Events &events);

private:
    HistoryServiceAdaptor *mAdaptor;
};

}

#endif // MANAGERDBUS_P_H
