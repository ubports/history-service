/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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
    void notifyThreadParticipantsChanged(const QVariantMap &thread,
                                   const QList<QVariantMap> &added,
                                   const QList<QVariantMap> &removed,
                                   const QList<QVariantMap> &modified);

    void notifyEventsAdded(const QList<QVariantMap> &events);
    void notifyEventsModified(const QList<QVariantMap> &events);
    void notifyEventsRemoved(const QList<QVariantMap> &events);

    // functions exposed on DBUS
    QVariantMap ThreadForParticipants(const QString &accountId,
                                      int type,
                                      const QStringList &participants,
                                      int matchFlags,
                                      bool create);
    QVariantMap ThreadForProperties(const QString &accountId,
                                    int type,
                                    const QVariantMap &properties,
                                    int matchFlags,
                                    bool create);
    QList<QVariantMap> ParticipantsForThreads(const QList<QVariantMap> &threadIds);
    bool WriteEvents(const QList <QVariantMap> &events);
    bool RemoveThreads(const QList <QVariantMap> &threads);
    bool RemoveEvents(const QList <QVariantMap> &events);
    bool RemoveEventsBy(int type, const QVariantMap &filter, const QVariantMap &sort);
    void MarkThreadsAsRead(const QList <QVariantMap> &threads);
    int EventsCount(int type, const QVariantMap &filter);

    // views
    QString QueryThreads(int type, const QVariantMap &sort, const QVariantMap &filter, const QVariantMap &properties);
    QString QueryEvents(int type, const QVariantMap &sort, const QVariantMap &filter);
    QVariantMap GetSingleThread(int type, const QString &accountId, const QString &threadId, const QVariantMap &properties);
    QVariantMap GetSingleEvent(int type, const QString &accountId, const QString &threadId, const QString &eventId);

Q_SIGNALS:
    // signals that will be relayed into the bus
    void ThreadsAdded(const QList<QVariantMap> &threads);
    void ThreadsModified(const QList<QVariantMap> &threads);
    void ThreadsRemoved(const QList<QVariantMap> &threads);
    void ThreadParticipantsChanged(const QVariantMap &thread,
                             const QList<QVariantMap> &added,
                             const QList<QVariantMap> &removed,
                             const QList<QVariantMap> &modified);

    void EventsAdded(const QList<QVariantMap> &events);
    void EventsModified(const QList<QVariantMap> &events);
    void EventsRemoved(const QList<QVariantMap> &events);

protected:
    void timerEvent(QTimerEvent *event) override;

protected Q_SLOTS:
    void filterDuplicatesAndAdd(QList<QVariantMap> &targetList, const QList<QVariantMap> newItems, const QStringList &propertiesToCompare);
    void triggerSignals();
    void processSignals();

private:
    HistoryServiceAdaptor *mAdaptor;
    QList<QVariantMap> mThreadsAdded;
    QList<QVariantMap> mThreadsModified;
    QList<QVariantMap> mThreadsRemoved;
    QList<QVariantMap> mEventsAdded;
    QList<QVariantMap> mEventsModified;
    QList<QVariantMap> mEventsRemoved;
    int mSignalsTimer;
};

#endif // HISTORYSERVICEDBUS_H
