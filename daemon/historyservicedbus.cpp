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

#include "historyservicedbus.h"
#include "historyserviceadaptor.h"
#include "types.h"
#include "event.h"
#include "itemfactory.h"
#include "manager.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"

Q_DECLARE_METATYPE(QList< QVariantMap >)

HistoryServiceDBus::HistoryServiceDBus(QObject *parent) :
    QObject(parent), mAdaptor(0)
{
    qDBusRegisterMetaType<QList<QVariantMap> >();
    connectToBus();
}

bool HistoryServiceDBus::connectToBus()
{
    bool ok = QDBusConnection::sessionBus().registerService(History::DBusService);
    if (!ok) {
        return false;
    }

    if (!mAdaptor) {
        mAdaptor = new HistoryServiceAdaptor(this);
    }

    return QDBusConnection::sessionBus().registerObject(History::DBusObjectPath, this);
}

void HistoryServiceDBus::notifyThreadsAdded(const QList<QVariantMap> &threads)
{
    Q_EMIT ThreadsAdded(threads);
}

void HistoryServiceDBus::notifyThreadsModified(const QList<QVariantMap> &threads)
{
    Q_EMIT ThreadsModified(threads);
}

void HistoryServiceDBus::notifyThreadsRemoved(const QList<QVariantMap> &threads)
{
    Q_EMIT ThreadsRemoved(threads);
}

void HistoryServiceDBus::notifyEventsAdded(const QList<QVariantMap> &events)
{
    Q_EMIT EventsAdded(events);
}

void HistoryServiceDBus::notifyEventsModified(const QList<QVariantMap> &events)
{
    Q_EMIT EventsModified(events);
}

void HistoryServiceDBus::notifyEventsRemoved(const QList<QVariantMap> &events)
{
    Q_EMIT EventsRemoved(events);
}

QVariantMap HistoryServiceDBus::ThreadForParticipants(const QString &accountId,
                                                      int type,
                                                      const QStringList &participants,
                                                      int matchFlags,
                                                      bool create)
{
    qDebug() << __PRETTY_FUNCTION__;
    qDebug() << accountId << type << participants << matchFlags << create;
    return QVariantMap();
}

