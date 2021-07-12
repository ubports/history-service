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
                                             const History::Sort &sort,
                                             const History::Filter &filter)
    : History::PluginEventView(),  mPlugin(plugin), mType(type), mSort(sort), mFilter(filter),
      mQuery(SQLiteDatabase::instance()->database()), mPageSize(15), mOffset(0), mValid(true)
{
    mTemporaryTable = QString("eventview%1%2").arg(QString::number((qulonglong)this), QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmsszzz"));
    mQuery.setForwardOnly(true);

    // FIXME: validate the filter
    QVariantMap filterValues;
    QString condition = mPlugin->filterToString(filter, filterValues);
    QString order;
    if (!sort.sortField().isNull()) {
        // WORKAROUND: Supports multiple fields by split it using ','
        Q_FOREACH(const QString& field, sort.sortField().split(",")) {
            order += QString("%1 %2, ")
                    .arg(field.trimmed())
                    .arg(sort.sortOrder() == Qt::AscendingOrder ? "ASC" : "DESC");
        }

        order = QString("ORDER BY %1").arg(order.mid(0, order.lastIndexOf(",")));
        // FIXME: check case sensitiviy
    }

    QString queryText = QString("CREATE TEMP TABLE %1 AS ").arg(mTemporaryTable);
    queryText += mPlugin->sqlQueryForEvents(type, condition, order);

    if (!mQuery.prepare(queryText)) {
        mValid = false;
        Q_EMIT Invalidated();
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return;
    }

    Q_FOREACH(const QString &key, filterValues.keys()) {
        mQuery.bindValue(key, filterValues[key]);
    }

    if (!mQuery.exec()) {
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

int SQLiteHistoryEventView::TotalCount()
{
    mQuery.prepare(QString("SELECT count(*) FROM %1").arg(mTemporaryTable));
    if (!mQuery.exec() || !mQuery.next()) {
        qWarning() << "Failed to get total count. Error:" << mQuery.lastError();
        return 0;
    }

    int totalCount = mQuery.value(0).toUInt();
    mQuery.clear();

    return totalCount;
}

bool SQLiteHistoryEventView::IsValid() const
{
    return mQuery.isActive();
}
