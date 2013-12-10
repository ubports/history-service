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

#ifndef HISTORYSERVICEDBUS_H
#define HISTORYSERVICEDBUS_H

#include <QDBusContext>
#include <QObject>
#include "types.h"

class HistoryServiceAdaptor;

class HistoryServiceDBus : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit HistoryServiceDBus(QObject *parent = 0);

    bool connectToBus();

    void notifyThreadsAdded(const QList<QVariantMap> &threads);
    void notifyThreadsModified(const QList<QVariantMap> &threads);
    void notifyThreadsRemoved(const QList<QVariantMap> &threads);

    void notifyEventsAdded(const QList<QVariantMap> &events);
    void notifyEventsModified(const QList<QVariantMap> &events);
    void notifyEventsRemoved(const QList<QVariantMap> &events);

    // functions exposed on DBUS
    QVariantMap ThreadForParticipants(const QString &accountId,
                                      int type,
                                      const QStringList &participants,
                                      int matchFlags,
                                      bool create);
    bool WriteEvents(const QList <QVariantMap> &events);
    bool RemoveThreads(const QList <QVariantMap> &threads);
    bool RemoveEvents(const QList <QVariantMap> &events);

    // views
    QString QueryThreads(int type, const QVariantMap &sort, const QVariantMap &filter);
    QString QueryEvents(int type, const QVariantMap &sort, const QVariantMap &filter);
    QVariantMap GetSingleThread(int type, const QString &accountId, const QString &threadId);
    QVariantMap GetSingleEvent(int type, const QString &accountId, const QString &threadId, const QString &eventId);

Q_SIGNALS:
    // signals that will be relayed into the bus
    void ThreadsAdded(const QList<QVariantMap> &threads);
    void ThreadsModified(const QList<QVariantMap> &threads);
    void ThreadsRemoved(const QList<QVariantMap> &threads);

    void EventsAdded(const QList<QVariantMap> &events);
    void EventsModified(const QList<QVariantMap> &events);
    void EventsRemoved(const QList<QVariantMap> &events);

private:
    HistoryServiceAdaptor *mAdaptor;
};

#endif // HISTORYSERVICEDBUS_H