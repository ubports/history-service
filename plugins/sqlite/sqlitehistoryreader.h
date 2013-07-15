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

#ifndef SQLITEHISTORYREADER_H
#define SQLITEHISTORYREADER_H

#include "reader.h"

class SQLiteHistoryReader : public History::Reader
{
    Q_OBJECT
public:
    explicit SQLiteHistoryReader(QObject *parent = 0);
    History::ThreadViewPtr queryThreads(History::EventType type,
                                        const History::SortPtr &sort = History::SortPtr(),
                                        const History::FilterPtr &filter = History::FilterPtr());
    History::EventViewPtr queryEvents(History::EventType type,
                                      const History::SortPtr &sort = History::SortPtr(),
                                      const History::FilterPtr &filter = History::FilterPtr());
};

#endif // SQLITEHISTORYREADER_H
