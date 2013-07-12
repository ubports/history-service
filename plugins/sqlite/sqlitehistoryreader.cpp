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

#include "sqlitehistoryreader.h"
#include "sqlitehistoryeventview.h"
#include "sqlitehistorythreadview.h"
#include "sqlitedatabase.h"
#include "types.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

SQLiteHistoryReader::SQLiteHistoryReader(QObject *parent) :
    History::Reader(parent)
{
}

History::ThreadViewPtr SQLiteHistoryReader::queryThreads(History::EventType type,
                                                         const History::SortPtr &sort,
                                                         const History::FilterPtr &filter)
{
    return History::ThreadViewPtr(new SQLiteHistoryThreadView(this, type, sort, filter));
}

History::EventViewPtr SQLiteHistoryReader::queryEvents(History::EventType type,
                                                       const History::SortPtr &sort,
                                                       const History::FilterPtr &filter)
{
    return History::EventViewPtr(new SQLiteHistoryEventView(this, type, sort, filter));
}
