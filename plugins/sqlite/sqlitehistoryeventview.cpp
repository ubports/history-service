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
#include "itemfactory.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDebug>
#include <QSqlError>

SQLiteHistoryEventView::SQLiteHistoryEventView(SQLiteHistoryReader *reader,
                                             History::EventType type,
                                             const History::SortPtr &sort,
                                             const History::FilterPtr &filter)
    : mType(type), mSort(sort), mFilter(filter), mQuery(SQLiteDatabase::instance()->database()),
      mPageSize(15), mReader(reader)
{
    // FIXME: sort the results properly
    Q_UNUSED(sort)

    mQuery.setForwardOnly(true);

    // FIXME: validate the filter
    QString condition;
    if (!filter.isNull()) {
        condition = filter->toString();
    }
    if (!condition.isEmpty()) {
        condition.prepend(" WHERE ");
    }

    QString queryText;

    switch (type) {
    case History::EventTypeText:
        queryText = QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent,"
                            "message, messageType, messageFlags, readTimestamp FROM text_events %1").arg(condition);
        break;
    case History::EventTypeVoice:
        queryText = QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent,"
                            "duration, missed FROM voice_events %1").arg(condition);
        break;
    }

    if (!mQuery.exec(queryText)) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return;
    }
}

QList<History::EventPtr> SQLiteHistoryEventView::nextPage()
{
    QList<History::EventPtr> events;
    int remaining = mPageSize;

    while (mQuery.next() && remaining-- > 0) {
        switch (mType) {
        case History::EventTypeText:
            events << History::ItemFactory::instance()->createTextEvent(mQuery.value(0).toString(),
                                                                        mQuery.value(1).toString(),
                                                                        mQuery.value(2).toString(),
                                                                        mQuery.value(3).toString(),
                                                                        mQuery.value(4).toDateTime(),
                                                                        mQuery.value(5).toBool(),
                                                                        mQuery.value(6).toString(),
                                                                        (History::MessageType) mQuery.value(7).toInt(),
                                                                        (History::MessageFlags) mQuery.value(8).toInt(),
                                                                        mQuery.value(9).toDateTime());
            break;
        case History::EventTypeVoice:
            events << History::ItemFactory::instance()->createVoiceEvent(mQuery.value(0).toString(),
                                                                         mQuery.value(1).toString(),
                                                                         mQuery.value(2).toString(),
                                                                         mQuery.value(3).toString(),
                                                                         mQuery.value(4).toDateTime(),
                                                                         mQuery.value(5).toBool(),
                                                                         mQuery.value(7).toBool(),
                                                                         QTime(0,0).addSecs(mQuery.value(6).toInt()));
            break;
        }
    }

    return events;
}

bool SQLiteHistoryEventView::isValid() const
{
    return mQuery.isActive();
}
