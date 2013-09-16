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

#ifndef SQLITEHISTORYEVENTVIEW_H
#define SQLITEHISTORYEVENTVIEW_H

#include "plugineventview.h"
#include "types.h"
#include <QSqlQuery>

class SQLiteHistoryPlugin;

class SQLiteHistoryEventView : public History::PluginEventView
{
    Q_OBJECT

public:
    SQLiteHistoryEventView(SQLiteHistoryPlugin *plugin,
                          History::EventType type,
                          const History::SortPtr &sort,
                          const QString &filter);
    ~SQLiteHistoryEventView();

    QList<QVariantMap> NextPage();
    bool IsValid() const;

protected:


private:
    History::EventType mType;
    History::SortPtr mSort;
    QString mFilter;
    QSqlQuery mQuery;
    int mPageSize;
    SQLiteHistoryPlugin *mPlugin;
    QString mTemporaryTable;
    int mOffset;
    bool mValid;
};

#endif // SQLITEHISTORYEVENTVIEW_H
