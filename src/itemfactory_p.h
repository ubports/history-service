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

#ifndef ITEMFACTORY_P_H
#define ITEMFACTORY_P_H

#include <QMap>
#include "types.h"

namespace History
{

class ItemFactoryPrivate
{
public:
    QMap<QString, ThreadWeakPtr> threads;
    QMap<QString, EventWeakPtr> events;

    QString hashItem(EventType type, const QString &accountId, const QString &threadId, const QString &eventId = QString::null);
    void cleanupThreads();
    void cleanupEvents();
};

}

#endif // ITEMFACTORY_P_H
