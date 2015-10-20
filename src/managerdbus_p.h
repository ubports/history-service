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

#include <QDBusInterface>
#include <QObject>
#include "types.h"
#include "event.h"
#include "thread.h"

class HistoryServiceAdaptor;

namespace History
{

class ManagerDBus : public QObject
{
    Q_OBJECT
public:
    explicit ManagerDBus(QObject *parent = 0);

    Thread threadForParticipants(const QString &accountId,
                                 EventType type,
                                 const QStringList &participants,
                                 History::MatchFlags matchFlags,
                                 bool create);

    bool writeEvents(const History::Events &events);
    bool removeThreads(const Threads &threads);
    bool removeEvents(const Events &events);
    Thread getSingleThread(EventType type, const QString &accountId, const QString &threadId, const QVariantMap &properties = QVariantMap());
    Event getSingleEvent(EventType type, const QString &accountId, const QString &threadId, const QString &eventId);

Q_SIGNALS:
    // signals that will be triggered after processing bus signals
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
    QList<QVariantMap> threadsToProperties(const Threads &threads);

    Event eventFromProperties(const QVariantMap &properties);
    Events eventsFromProperties(const QList<QVariantMap> &eventsProperties);
    QList<QVariantMap> eventsToProperties(const Events &events);

private:
    HistoryServiceAdaptor *mAdaptor;
    QDBusInterface mInterface;
};

}

#endif // MANAGERDBUS_P_H
