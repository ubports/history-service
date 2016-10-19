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

#ifndef HISTORY_THREAD_H
#define HISTORY_THREAD_H

#include <QDBusArgument>
#include <QDateTime>
#include <QScopedPointer>
#include <QStringList>
#include <QVariantMap>
#include "types.h"
#include "event.h"
#include "participant.h"

namespace History
{

class ThreadPrivate;
class ItemFactory;
class Thread;

typedef QList<Thread> Threads;

class Thread
{
    Q_DECLARE_PRIVATE(Thread)
    friend class ItemFactory;

public:
    explicit Thread();
    Thread(const QString &accountId,
           const QString &threadId,
           EventType type,
           const Participants &participants,
           const QDateTime &timestamp = QDateTime(),
           const Event &lastEvent = Event(),
           int count = 0,
           int unreadCount = 0,
           const Threads &groupedThreads = Threads(),
           ChatType chatType = ChatTypeNone,
           const QVariantMap &chatRoomInfo = QVariantMap());
    Thread(const Thread &other);
    virtual ~Thread();
    Thread& operator=(const Thread &other);

    QString accountId() const;
    QString threadId() const;
    EventType type() const;
    Participants participants() const;
    QDateTime timestamp() const;
    Event lastEvent() const;
    int count() const;
    int unreadCount() const;
    ChatType chatType() const;
    Threads groupedThreads() const;
    QVariantMap chatRoomInfo() const;

    bool isNull() const;
    bool operator==(const Thread &other) const;
    bool operator<(const Thread &other) const;

    virtual QVariantMap properties() const;

    static Thread fromProperties(const QVariantMap &properties);

protected:
    QSharedPointer<ThreadPrivate> d_ptr;
};

const QDBusArgument &operator>>(const QDBusArgument &argument, Threads &threads);

}

#endif // HISTORY_THREAD_H
