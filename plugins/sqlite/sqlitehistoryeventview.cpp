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
#include "filter.h"
#include "sort.h"
#include "itemfactory.h"
#include "textevent.h"
#include "texteventattachment.h"
#include "voiceevent.h"
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
    if (!condition.isEmpty()) {
        condition.prepend(" WHERE ");
    }

    QString order;
    if (!sort.isNull() && !sort->sortField().isNull()) {
        order = QString("ORDER BY %1 %2").arg(sort->sortField(), sort->sortOrder() == Qt::AscendingOrder ? "ASC" : "DESC");
        // FIXME: check case sensitiviy
    }

    QString queryText = QString("CREATE TEMP TABLE %1 AS ").arg(mTemporaryTable);

    switch (type) {
    case History::EventTypeText:
        queryText += QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent,"
                             "message, messageType, messageFlags, readTimestamp, subject FROM text_events %1 %2").arg(condition, order);
        break;
    case History::EventTypeVoice:
        queryText += QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent,"
                             "duration, missed FROM voice_events %1 %2").arg(condition, order);
        break;
    }

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

    while (mQuery.next()) {
        QVariantMap event;
        History::MessageType messageType;
        event[History::FieldAccountId] = mQuery.value(0);
        event[History::FieldThreadId] = mQuery.value(1);
        event[History::FieldEventId] = mQuery.value(2);
        event[History::FieldSenderId] = mQuery.value(3);
        event[History::FieldTimestamp] = mQuery.value(4);
        event[History::FieldNewEvent] = mQuery.value(5);

        switch (mType) {
        case History::EventTypeText:
            messageType = (History::MessageType) mQuery.value(7).toInt();
            if (messageType == History::MessageTypeMultiParty)  {
                QSqlQuery attachmentsQuery(SQLiteDatabase::instance()->database());
                attachmentsQuery.prepare("SELECT attachmentId, contentType, filePath, status FROM text_event_attachments "
                                    "WHERE accountId=:accountId and threadId=:threadId and eventId=:eventId");
                attachmentsQuery.bindValue(":accountId", event[History::FieldAccountId]);
                attachmentsQuery.bindValue(":threadId", event[History::FieldThreadId]);
                attachmentsQuery.bindValue(":eventId", event[History::FieldEventId]);
                if (!attachmentsQuery.exec()) {
                    qCritical() << "Error:" << attachmentsQuery.lastError() << attachmentsQuery.lastQuery();
                }
                // FIXME: reimplement
                /*while (attachmentsQuery.next()) {
                    History::TextEventAttachmentPtr attachment = History::TextEventAttachmentPtr(
                                new History::TextEventAttachment(accountId,
                                                                 threadId,
                                                                 eventId,
                                                                 attachmentsQuery.value(0).toString(),
                                                                 attachmentsQuery.value(1).toString(),
                                                                 attachmentsQuery.value(2).toString(),
                                                                 (History::AttachmentFlag) attachmentsQuery.value(3).toInt()));
                    attachments << attachment;

                }*/
                attachmentsQuery.clear();
            }
            event[History::FieldMessage] = mQuery.value(6);
            event[History::FieldMessageType] = mQuery.value(7);
            event[History::FieldMessageFlags] = mQuery.value(8);
            event[History::FieldReadTimestamp] = mQuery.value(9);
            break;
        case History::EventTypeVoice:
            event[History::FieldDuration] = QTime(0,0).addSecs(mQuery.value(6).toInt());
            event[History::FieldMissed] = mQuery.value(7);
            break;
        }

        events << event;
    }

    mOffset += mPageSize;
    mQuery.clear();

    return events;
}

bool SQLiteHistoryEventView::IsValid() const
{
    return mQuery.isActive();
}
