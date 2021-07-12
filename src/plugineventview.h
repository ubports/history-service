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

#ifndef PLUGINEVENTVIEW_H
#define PLUGINEVENTVIEW_H

#include <QObject>
#include <QDBusContext>
#include <QScopedPointer>
#include <QVariantMap>

namespace History {

class PluginEventViewPrivate;

class PluginEventView : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PluginEventView)
public:
    explicit PluginEventView(QObject *parent = 0);
    virtual ~PluginEventView();

    // DBus exposed methods
    Q_NOREPLY void Destroy();
    virtual QList<QVariantMap> NextPage() = 0;
    virtual bool IsValid() const;
    virtual int TotalCount();

    // other methods
    QString objectPath() const;

Q_SIGNALS:
    void Invalidated();

private:
    QScopedPointer<PluginEventViewPrivate> d_ptr;
};

}

#endif // PLUGINEVENTVIEW_H
