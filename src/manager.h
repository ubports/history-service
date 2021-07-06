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

#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <QObject>
#include <QString>
#include "types.h"
#include "event.h"
#include "filter.h"
#include "sort.h"
#include "thread.h"

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
                               const Sort &sort = Sort(),
                               const Filter &filter = Filter(),
                               const QVariantMap &properties = QVariantMap());

    EventViewPtr queryEvents(EventType type,
                             const Sort &sort = Sort(),
                             const Filter &filter = Filter());

    Event getSingleEvent(EventType type, const QString &accountId, const QString &threadId, const QString &eventId);

    Thread threadForParticipants(const QString &accountId,
                                 EventType type,
                                 const QStringList &participants,
                                 History::MatchFlags matchFlags = History::MatchCaseSensitive,
                                 bool create = false);
    Thread threadForProperties(const QString &accountId,
                               EventType type,
                               const QVariantMap &properties,
                               History::MatchFlags matchFlags = History::MatchCaseSensitive,
                               bool create = false);
    void requestThreadParticipants(const History::Threads &threads);
    Thread getSingleThread(EventType type, const QString &accountId, const QString &threadId, const QVariantMap &properties = QVariantMap());

    bool writeEvents(const History::Events &events);
    bool removeThreads(const Threads &threads);
    bool removeEvents(const Events &events);
    bool removeEvents(EventType type, const Filter &filter, const Sort &sort);
    int eventsCount(EventType type, const Filter &filter);
    void markThreadsAsRead(const History::Threads &thread);

    bool isServiceRunning() const;

Q_SIGNALS:
    void threadsAdded(const History::Threads &threads);
    void threadsModified(const History::Threads &threads);
    void threadsRemoved(const History::Threads &threads);
    void threadParticipantsChanged(const History::Thread &thread, const History::Participants &added, const History::Participants &removed, const History::Participants &modified);

    void eventsAdded(const History::Events &events);
    void eventsModified(const History::Events &events);
    void eventsRemoved(const History::Events &events);

    void serviceRunningChanged();

private:
    Manager();
    QScopedPointer<ManagerPrivate> d_ptr;
};

}

#endif
