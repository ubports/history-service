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

#include "threadview.h"
#include "threadview_p.h"
#include "filter.h"
#include "manager.h"
#include "sort.h"
#include "thread.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

namespace History
{

// ------------- ThreadViewPrivate ------------------------------------------------

ThreadViewPrivate::ThreadViewPrivate(History::EventType theType,
                                     const History::Sort &theSort,
                                     const History::Filter &theFilter)
    : type(theType), sort(theSort), filter(theFilter), valid(true), dbus(0)
{
}

Threads ThreadViewPrivate::filteredThreads(const Threads &threads)
{
    bool filterNull = filter.isNull();

    Threads filtered;
    Q_FOREACH(const Thread &thread, threads) {
        if (thread.type() != type) {
            continue;
        }

        if (filterNull || filter.match(thread.properties())) {
            filtered << thread;
        }
    }

    return filtered;
}

void ThreadViewPrivate::_d_threadsAdded(const History::Threads &threads)
{
    Q_Q(ThreadView);

    Threads filtered = filteredThreads(threads);
    if (!filtered.isEmpty()) {
        Q_EMIT q->threadsAdded(filtered);
    }
}

void ThreadViewPrivate::_d_threadsModified(const Threads &threads)
{
    Q_Q(ThreadView);

    Threads filtered = filteredThreads(threads);
    if (!filtered.isEmpty()) {
        Q_EMIT q->threadsModified(filtered);
    }
}

void ThreadViewPrivate::_d_threadsRemoved(const Threads &threads)
{
    Q_Q(ThreadView);

    Threads filtered = filteredThreads(threads);
    if (!filtered.isEmpty()) {
        Q_EMIT q->threadsRemoved(filtered);
    }
}

// ------------- ThreadView -------------------------------------------------------

ThreadView::ThreadView(History::EventType type,
                       const History::Sort &sort,
                       const Filter &filter,
                       const QVariantMap &properties)
    : d_ptr(new ThreadViewPrivate(type, sort, filter))
{
    d_ptr->q_ptr = this;

    if (!Manager::instance()->isServiceRunning()) {
        Q_EMIT invalidated();
        d_ptr->valid = false;
        return;
    }

    QDBusInterface interface(History::DBusService, History::DBusObjectPath, History::DBusInterface);

    QDBusReply<QString> reply = interface.call("QueryThreads",
                                               (int) type,
                                               sort.properties(),
                                               filter.properties(),
                                               properties);
    if (!reply.isValid()) {
        Q_EMIT invalidated();
        d_ptr->valid = false;
        return;
    }

    d_ptr->objectPath = reply.value();

    d_ptr->dbus = new QDBusInterface(History::DBusService, d_ptr->objectPath, History::ThreadViewInterface,
                                     QDBusConnection::sessionBus(), this);

    connect(Manager::instance(),
            SIGNAL(threadsAdded(History::Threads)),
            SLOT(_d_threadsAdded(History::Threads)));
    connect(Manager::instance(),
            SIGNAL(threadsModified(History::Threads)),
            SLOT(_d_threadsModified(History::Threads)));
    connect(Manager::instance(),
            SIGNAL(threadsRemoved(History::Threads)),
            SLOT(_d_threadsRemoved(History::Threads)));
}

ThreadView::~ThreadView()
{
    Q_D(ThreadView);
    if (d->valid) {
        d->dbus->call("Destroy");
    }
}

Threads ThreadView::nextPage()
{
    Threads threads;
    Q_D(ThreadView);
    if (!d->valid) {
        return threads;
    }

    QDBusReply<QList<QVariantMap> > reply = d->dbus->call("NextPage");

    if (!reply.isValid()) {
        qDebug() << "Error:" << reply.error();
        d->valid = false;
        Q_EMIT invalidated();
        return threads;
    }

    QList<QVariantMap> threadsProperties = reply.value();
    Q_FOREACH(const QVariantMap &properties, threadsProperties) {
        Thread thread = Thread::fromProperties(properties);
        if (!thread.isNull()) {
            threads << thread;
        }
    }

    return threads;
}

bool ThreadView::isValid() const
{
    Q_D(const ThreadView);
    return d->valid;
}

}

#include "moc_threadview.cpp"
