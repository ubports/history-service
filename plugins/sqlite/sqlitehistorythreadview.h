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

#ifndef SQLITEHISTORYTHREADVIEW_H
#define SQLITEHISTORYTHREADVIEW_H

#include "pluginthreadview.h"
#include "filter.h"
#include "types.h"
#include "sort.h"
#include <QSqlQuery>

class SQLiteHistoryPlugin;

class SQLiteHistoryThreadView : public History::PluginThreadView
{
    Q_OBJECT
public:
    SQLiteHistoryThreadView(SQLiteHistoryPlugin *plugin,
                            History::EventType type,
                            const History::Sort &sort,
                            const History::Filter &filter,
                            const QVariantMap &properties);
    ~SQLiteHistoryThreadView();

    QList<QVariantMap> NextPage();
    bool IsValid() const;

private:
    History::EventType mType;
    History::Sort mSort;
    History::Filter mFilter;
    QSqlQuery mQuery;
    int mPageSize;
    SQLiteHistoryPlugin *mPlugin;
    QString mTemporaryTable;
    int mOffset;
    bool mValid;
    QVariantMap mQueryProperties;
};

#endif // SQLITEHISTORYTHREADVIEW_H
