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

#ifndef HISTORY_READER_H
#define HISTORY_READER_H

#include <QObject>
#include "types.h"

namespace History
{

class Reader : public QObject
{
    Q_OBJECT
public:
    explicit Reader(QObject *parent = 0) : QObject(parent) {}
    virtual ~Reader() {}

    virtual ThreadViewPtr queryThreads(EventType type,
                                       const SortPtr &sort = SortPtr(),
                                       const FilterPtr &filter = FilterPtr()) = 0;
    virtual EventViewPtr queryEvents(EventType type,
                                     const SortPtr &sort = SortPtr(),
                                     const FilterPtr &filter = FilterPtr()) = 0;
    virtual ThreadPtr getSingleThread(EventType type,
                                      const QString &accountId,
                                      const QString &threadId) = 0;
    virtual ThreadPtr threadForParticipants(const QString &accountId, EventType type, const QStringList &participants) = 0;
};

}
#endif // HISTORY_READER_H
