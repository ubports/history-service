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
#include "filter.h"
#include "intersectionfilter.h"
#include "types.h"
#include "thread.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

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

History::ThreadPtr SQLiteHistoryReader::threadForParticipants(const QString &accountId, History::EventType type, const QStringList &participants)
{
    if (participants.isEmpty()) {
        return History::ThreadPtr(0);
    }

    QSqlQuery query(SQLiteDatabase::instance()->database());

    // select all the threads the first participant is listed in, and from that list
    // check if any of the threads has all the other participants listed
    // FIXME: find a better way to do this
    query.prepare("SELECT threadId FROM thread_participants WHERE "
                  "participantId=:participantId AND type=:type AND accountId=:accountId");
    query.bindValue(":participantId", participants[0]);
    query.bindValue(":type", type);
    query.bindValue(":accountId", accountId);
    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return History::ThreadPtr(0);
    }

    QStringList threadIds;
    while (query.next()) {
        threadIds << query.value(0).toString();
    }

    QString existingThread;
    // now for each threadId, check if all the other participants are listed
    Q_FOREACH(const QString &threadId, threadIds) {
        query.prepare("SELECT participantId FROM thread_participants WHERE "
                      "threadId=:threadId AND type=:type AND accountId=:accountId");
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        query.bindValue(":accountId", accountId);
        if (!query.exec()) {
            qCritical() << "Error:" << query.lastError() << query.lastQuery();
            return History::ThreadPtr(0);
        }

        QStringList threadParticipants;
        while (query.next()) {
            threadParticipants << query.value(0).toString();
        }

        // and now compare the lists
        bool found = true;
        Q_FOREACH(const QString &participant, participants) {
            if (!threadParticipants.contains(participant)) {
                found = false;
                break;
            }
        }

        if (found) {
            existingThread = threadId;
            break;
        }
    }

    if (!existingThread.isNull()) {
        return History::ThreadPtr(new History::Thread(accountId, existingThread, type, participants));
    }

    return History::ThreadPtr();
}

History::ThreadPtr SQLiteHistoryReader::getSingleThread(History::EventType type, const QString &accountId, const QString &threadId)
{
    History::IntersectionFilterPtr intersectionFilter(new History::IntersectionFilter());
    intersectionFilter->append(History::FilterPtr(new History::Filter("accountId", accountId)));
    intersectionFilter->append(History::FilterPtr(new History::Filter("threadId", threadId)));
    History::ThreadViewPtr view = queryThreads(type, History::SortPtr(), intersectionFilter);

    History::Threads threads= view->nextPage();
    History::ThreadPtr thread;
    if (!threads.isEmpty()) {
        thread = threads.first();
    }

    return thread;
}
