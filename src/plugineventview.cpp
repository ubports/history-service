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

#include "plugineventview.h"
#include "plugineventview_p.h"
#include "plugineventviewadaptor.h"
#include "types.h"
#include <QDBusConnection>
#include <QDebug>

Q_DECLARE_METATYPE(QList< QVariantMap >)

namespace History {

PluginEventViewPrivate::PluginEventViewPrivate()
    : adaptor(0)
{
}

PluginEventView::PluginEventView(QObject *parent) :
    QObject(parent), d_ptr(new PluginEventViewPrivate())
{
    Q_D(PluginEventView);
    qDBusRegisterMetaType<QList<QVariantMap> >();

    d->adaptor = new EventViewAdaptor(this);

    QString id = QString("eventview%1%2").arg(QString::number((qulonglong)this), QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmsszzz"));
    d->objectPath = QString("%1/%2").arg(History::DBusObjectPath, id);
    QDBusConnection::sessionBus().registerObject(d->objectPath, this);
}

PluginEventView::~PluginEventView()
{
    Q_D(PluginEventView);
    QDBusConnection::sessionBus().unregisterObject(d->objectPath);
}

void PluginEventView::Destroy()
{
    Q_D(PluginEventView);
    deleteLater();
}

bool PluginEventView::IsValid() const
{
    return true;
}

int PluginEventView::TotalCount()
{
    return 0;
}

QString PluginEventView::objectPath() const
{
    Q_D(const PluginEventView);
    return d->objectPath;
}

}
