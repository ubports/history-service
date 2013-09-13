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

#include "sqlitehistorythreadview.h"
#include "sqlitedatabase.h"
#include "thread.h"
#include "sort.h"
#include "intersectionfilter.h"
#include "itemfactory.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDebug>
#include <QSqlError>

SQLiteHistoryThreadView::SQLiteHistoryThreadView(SQLiteHistoryPlugin *plugin,
                                                 History::EventType type,
                                                 const History::SortPtr &sort,
                                                 const QString &filter)
    : History::PluginThreadView(), mPlugin(plugin), mType(type), mSort(sort),
      mFilter(filter), mPageSize(15), mQuery(SQLiteDatabase::instance()->database()), mOffset(0), mValid(true)
{
    qDebug() << __PRETTY_FUNCTION__;
    mTemporaryTable = QString("threadview%1%2").arg(QString::number((qulonglong)this), QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmsszzz"));
    mQuery.setForwardOnly(true);

    // FIXME: validate the filter
    QString condition = filter;
    if (!filter.isNull()) {
        // FIXME: the filters should be implemented in a better way
        condition.replace("accountId=", "threads.accountId=");
        condition.replace("threadId=", "threads.threadId=");
        condition.replace("count=", "threads.count=");
        condition.replace("unreadCount=", "threads.unreadCount=");
    }

    if (!condition.isEmpty()) {
        condition.prepend(" AND ");
    }

    QString order;
    if (!sort.isNull() && !sort->sortField().isNull()) {
        order = QString("ORDER BY threads.%1 %2").arg(sort->sortField(), sort->sortOrder() == Qt::AscendingOrder ? "ASC" : "DESC");
        // FIXME: check case sensitiviy
    }

    QStringList fields;
    fields << "threads.accountId"
           << "threads.threadId"
           << "threads.lastEventId"
           << "threads.count"
           << "threads.unreadCount";

    // get the participants in the query already
    fields << "(SELECT group_concat(thread_participants.participantId,  \"|,|\") "
              "FROM thread_participants WHERE thread_participants.accountId=threads.accountId "
              "AND thread_participants.threadId=threads.threadId "
              "AND thread_participants.type=threads.type GROUP BY accountId,threadId,type) as participants";

    QStringList extraFields;
    QString table;

    switch (type) {
    case History::EventTypeText:
        table = "text_events";
        extraFields << "text_events.message" << "text_events.messageType" << "text_events.messageFlags" << "text_events.readTimestamp";
        break;
    case History::EventTypeVoice:
        table = "voice_events";
        extraFields << "voice_events.duration" << "voice_events.missed";
        break;
    }

    fields << QString("%1.senderId").arg(table)
           << QString("%1.timestamp").arg(table)
           << QString("%1.newEvent").arg(table);
    fields << extraFields;

    QString queryText = QString("CREATE TEMP TABLE %1 AS ").arg(mTemporaryTable);
    queryText += QString("SELECT %1 FROM threads LEFT JOIN %2 ON threads.threadId=%2.threadId AND "
                         "threads.accountId=%2.accountId AND threads.lastEventId=%2.eventId WHERE threads.type=%3 %4 %5")
                         .arg(fields.join(", "), table, QString::number((int)type), condition, order);

    // create the temporary table
    if (!mQuery.exec(queryText)) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        mValid = false;
        Q_EMIT Invalidated();
        return;
    }
}

SQLiteHistoryThreadView::~SQLiteHistoryThreadView()
{
    if (!mQuery.exec(QString("DROP TABLE IF EXISTS %1").arg(mTemporaryTable))) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return;
    }
}

QList<QVariantMap> SQLiteHistoryThreadView::NextPage()
{
    qDebug() << __PRETTY_FUNCTION__;
    QList<QVariantMap> threads;

    // now prepare for selecting from it
    mQuery.prepare(QString("SELECT * FROM %1 LIMIT %2 OFFSET %3").arg(mTemporaryTable,
                                                                      QString::number(mPageSize), QString::number(mOffset)));
    if (!mQuery.exec()) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        mValid = false;
        Q_EMIT Invalidated();
        return threads;
    }

    while (mQuery.next()) {
        QVariantMap thread;
        thread[History::FieldAccountId] = mQuery.value(0);
        thread[History::FieldThreadId] = mQuery.value(1);

        thread[History::FieldEventId] = mQuery.value(2);
        thread[History::FieldCount] = mQuery.value(3);
        thread[History::FieldUnreadCount] = mQuery.value(4);
        thread[History::FieldParticipants] = mQuery.value(5).toString().split("|,|");

        // the generic event fields
        thread[History::FieldSenderId] = mQuery.value(6);
        thread[History::FieldTimestamp] = mQuery.value(7);
        thread[History::FieldNewEvent] = mQuery.value(8);

        // the next step is to get the last event
        switch (mType) {
        case History::EventTypeText:
            thread[History::FieldMessage] = mQuery.value(9);
            thread[History::FieldMessageType] = mQuery.value(10);
            thread[History::FieldMessageFlags] = mQuery.value(11);
            thread[History::FieldReadTimestamp] = mQuery.value(12);
            break;
        case History::EventTypeVoice:
            thread[History::FieldMissed] = mQuery.value(10);
            thread[History::FieldDuration] = QTime(0,0).addSecs(mQuery.value(9).toInt());
            break;
        }
        threads << thread;
    }

    mOffset += mPageSize;
    mQuery.clear();

    return threads;
}

bool SQLiteHistoryThreadView::IsValid() const
{
    return mValid;
}
