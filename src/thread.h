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

#ifndef HISTORY_THREAD_H
#define HISTORY_THREAD_H

#include <QDateTime>
#include <QScopedPointer>
#include <QStringList>
#include <QVariantMap>
#include "types.h"
#include "event.h"

namespace History
{

class ThreadPrivate;
class ItemFactory;

class Thread
{
    Q_DECLARE_PRIVATE(Thread)
    friend class ItemFactory;

public:
    explicit Thread();
    Thread(const QString &accountId,
           const QString &threadId,
           EventType type,
           const QStringList &participants,
           const Event &lastEvent = Event(),
           int count = 0,
           int unreadCount = 0);
    Thread(const Thread &other);
    virtual ~Thread();
    Thread& operator=(const Thread &other);

    QString accountId() const;
    QString threadId() const;
    EventType type() const;
    QStringList participants() const;
    Event lastEvent() const;
    int count() const;
    int unreadCount() const;

    bool isNull() const;
    bool operator==(const Thread &other) const;
    bool operator<(const Thread &other) const;

    virtual QVariantMap properties() const;

    static Thread fromProperties(const QVariantMap &properties);

protected:
    QSharedPointer<ThreadPrivate> d_ptr;
};

typedef QList<Thread> Threads;

}

#endif // HISTORY_THREAD_H
