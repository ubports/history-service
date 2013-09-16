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

#include "sqlitehistoryeventview.h"
#include "sqlitedatabase.h"
#include "sqlitehistoryplugin.h"
#include "sort.h"
#include <QDateTime>
#include <QDebug>
#include <QSqlError>

SQLiteHistoryEventView::SQLiteHistoryEventView(SQLiteHistoryPlugin *plugin,
                                             History::EventType type,
                                             const History::SortPtr &sort,
                                             const QString &filter)
    : History::PluginEventView(), mType(type), mSort(sort), mFilter(filter),
      mQuery(SQLiteDatabase::instance()->database()), mPageSize(15), mPlugin(plugin), mOffset(0), mValid(true)
{
    mTemporaryTable = QString("eventview%1%2").arg(QString::number((qulonglong)this), QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmsszzz"));
    mQuery.setForwardOnly(true);

    // FIXME: validate the filter
    QString condition = filter;
    QString order;
    if (!sort.isNull() && !sort->sortField().isNull()) {
        order = QString("ORDER BY %1 %2").arg(sort->sortField(), sort->sortOrder() == Qt::AscendingOrder ? "ASC" : "DESC");
        // FIXME: check case sensitiviy
    }

    QString queryText = QString("CREATE TEMP TABLE %1 AS ").arg(mTemporaryTable);
    queryText += mPlugin->sqlQueryForEvents(type, condition, order);

    if (!mQuery.exec(queryText)) {
        mValid = false;
        Q_EMIT Invalidated();
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return;
    }
}

SQLiteHistoryEventView::~SQLiteHistoryEventView()
{
    if (!mQuery.exec(QString("DROP TABLE IF EXISTS %1").arg(mTemporaryTable))) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return;
    }
}

QList<QVariantMap> SQLiteHistoryEventView::NextPage()
{
    QList<QVariantMap> events;

    // now prepare for selecting from it
    mQuery.prepare(QString("SELECT * FROM %1 LIMIT %2 OFFSET %3").arg(mTemporaryTable,
                                                                      QString::number(mPageSize), QString::number(mOffset)));
    if (!mQuery.exec()) {
        mValid = false;
        Q_EMIT Invalidated();
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return events;
    }

    events = mPlugin->parseEventResults(mType, mQuery);
    mOffset += mPageSize;
    mQuery.clear();

    return events;
}

bool SQLiteHistoryEventView::IsValid() const
{
    return mQuery.isActive();
}
