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

SQLiteHistoryEventView::SQLiteHistoryEventView(SQLiteHistoryReader *reader,
                                             History::EventType type,
                                             const History::SortPtr &sort,
                                             const History::FilterPtr &filter)
    : History::EventView(type, sort, filter), mType(type), mSort(sort), mFilter(filter),
      mQuery(SQLiteDatabase::instance()->database()), mPageSize(15), mReader(reader), mOffset(0)
{
    mTemporaryTable = QString("eventview%1%2").arg(QString::number((qulonglong)this), QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmsszzz"));
    mQuery.setForwardOnly(true);

    // FIXME: validate the filter
    QString condition;
    if (!filter.isNull()) {
        condition = filter->toString();
    }
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
                             "message, messageType, messageFlags, readTimestamp FROM text_events %1 %2").arg(condition, order);
        break;
    case History::EventTypeVoice:
        queryText += QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent,"
                             "duration, missed FROM voice_events %1 %2").arg(condition, order);
        break;
    }

    if (!mQuery.exec(queryText)) {
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

History::Events SQLiteHistoryEventView::nextPage()
{
    History::Events events;

    // now prepare for selecting from it
    mQuery.prepare(QString("SELECT * FROM %1 LIMIT %2 OFFSET %3").arg(mTemporaryTable,
                                                                      QString::number(mPageSize), QString::number(mOffset)));
    if (!mQuery.exec()) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return events;
    }

    while (mQuery.next()) {
        History::TextEventAttachments attachments;
        History::MessageType messageType;
        QString accountId = mQuery.value(0).toString();
        QString threadId = mQuery.value(1).toString();
        QString eventId = mQuery.value(2).toString();
        switch (mType) {
        case History::EventTypeText:
            messageType = (History::MessageType) mQuery.value(7).toInt();
            if (messageType == History::MultiPartMessage)  {
                QSqlQuery attachmentsQuery(SQLiteDatabase::instance()->database());
                attachmentsQuery.prepare("SELECT attachmentId, contentType, filePath FROM text_event_attachments "
                                    "WHERE accountId=:accountId and threadId=:threadId and eventId=:eventId");
                attachmentsQuery.bindValue(":accountId", accountId);
                attachmentsQuery.bindValue(":threadId", threadId);
                attachmentsQuery.bindValue(":eventId", eventId);
                if (!attachmentsQuery.exec()) {
                    qCritical() << "Error:" << attachmentsQuery.lastError() << attachmentsQuery.lastQuery();
                }
                while (attachmentsQuery.next()) {
                    History::TextEventAttachmentPtr attachment = History::TextEventAttachmentPtr(
                                new History::TextEventAttachment(accountId,
                                                                 threadId,
                                                                 eventId,
                                                                 attachmentsQuery.value("attachmentId").toString(),
                                                                 attachmentsQuery.value("contentType").toString(),
                                                                 attachmentsQuery.value("filePath").toString()));
                    attachments << attachment;

                }
                attachmentsQuery.clear();
            }
            events << History::ItemFactory::instance()->createTextEvent(accountId,
                                                                        threadId,
                                                                        eventId,
                                                                        mQuery.value(3).toString(),
                                                                        mQuery.value(4).toDateTime(),
                                                                        mQuery.value(5).toBool(),
                                                                        mQuery.value(6).toString(),
                                                                        messageType,
                                                                        (History::MessageFlags) mQuery.value(8).toInt(),
                                                                        mQuery.value(9).toDateTime(),
                                                                        attachments);
            break;
        case History::EventTypeVoice:
            events << History::ItemFactory::instance()->createVoiceEvent(accountId,
                                                                         threadId,
                                                                         eventId,
                                                                         mQuery.value(3).toString(),
                                                                         mQuery.value(4).toDateTime(),
                                                                         mQuery.value(5).toBool(),
                                                                         mQuery.value(7).toBool(),
                                                                         QTime(0,0).addSecs(mQuery.value(6).toInt()));
            break;
        }
    }

    mOffset += mPageSize;
    mQuery.clear();

    return events;
}

bool SQLiteHistoryEventView::isValid() const
{
    return mQuery.isActive();
}
