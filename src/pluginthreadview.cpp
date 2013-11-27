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

#include "pluginthreadview.h"
#include "pluginthreadview_p.h"
#include "pluginthreadviewadaptor.h"
#include "types.h"
#include <QDBusConnection>
#include <QDebug>

Q_DECLARE_METATYPE(QList< QVariantMap >)

namespace History {

PluginThreadViewPrivate::PluginThreadViewPrivate()
    : adaptor(0)
{
}

PluginThreadView::PluginThreadView(QObject *parent) :
    QObject(parent), d_ptr(new PluginThreadViewPrivate())
{
    Q_D(PluginThreadView);
    qDBusRegisterMetaType<QList<QVariantMap> >();

    d->adaptor = new ThreadViewAdaptor(this);

    QString id = QString("threadview%1%2").arg(QString::number((qulonglong)this), QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmsszzz"));
    d->objectPath = QString("%1/%2").arg(History::DBusObjectPath, id);
    QDBusConnection::sessionBus().registerObject(d->objectPath, this);
}

PluginThreadView::~PluginThreadView()
{
    Q_D(PluginThreadView);
    QDBusConnection::sessionBus().unregisterObject(d->objectPath);
}

void PluginThreadView::Destroy()
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_D(PluginThreadView);
    deleteLater();
}

bool PluginThreadView::IsValid() const
{
    return true;
}

QString PluginThreadView::objectPath() const
{
    Q_D(const PluginThreadView);
    return d->objectPath;
}

}
