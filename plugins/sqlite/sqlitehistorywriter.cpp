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

#include "sqlitehistorywriter.h"
#include "sqlitedatabase.h"
#include "itemfactory.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDebug>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>

SQLiteHistoryWriter::SQLiteHistoryWriter(QObject *parent) :
    History::Writer(parent)
{
    SQLiteDatabase::instance();
}

History::ThreadPtr SQLiteHistoryWriter::createThreadForParticipants(const QString &accountId, History::EventType type, const QStringList &participants)
{
    // WARNING: this function does NOT test to check if the thread is already created, you should check using HistoryReader::threadForParticipants()

    // Create a new thread
    // FIXME: define what the threadId will be
    QString threadId = participants.join("%");

    QSqlQuery query(SQLiteDatabase::instance()->database());
    query.prepare("INSERT INTO threads (accountId, threadId, type, count, unreadCount)"
                  "VALUES (:accountId, :threadId, :type, :count, :unreadCount)");
    query.bindValue(":accountId", accountId);
    query.bindValue(":threadId", threadId);
    query.bindValue(":type", type);
    query.bindValue(":count", 0);
    query.bindValue(":unreadCount", 0);
    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return History::ThreadPtr(0);
    }

    // and insert the participants
    Q_FOREACH(const QString &participant, participants) {
        query.prepare("INSERT INTO thread_participants (accountId, threadId, type, participantId)"
                      "VALUES (:accountId, :threadId, :type, :participantId)");
        query.bindValue(":accountId", accountId);
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        query.bindValue(":participantId", participant);
        if (!query.exec()) {
            qCritical() << "Error:" << query.lastError() << query.lastQuery();
            return History::ThreadPtr(0);
        }
    }

    // and finally create the thread
    History::ThreadPtr thread = History::ItemFactory::instance()->createThread(accountId, threadId, type, participants);
    return thread;
}

bool SQLiteHistoryWriter::removeThread(const History::ThreadPtr &thread)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    query.prepare("DELETE FROM threads WHERE accountId=:accountId AND threadId=:threadId AND type=:type");
    query.bindValue(":accountId", thread->accountId());
    query.bindValue(":threadId", thread->threadId());
    query.bindValue(":type", thread->type());

    if (!query.exec()) {
        qCritical() << "Failed to remove the thread: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryWriter::writeTextEvent(const History::TextEventPtr &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    // FIXME: add support for checking if an event already exists

    query.prepare("INSERT INTO text_events (accountId, threadId, eventId, senderId, timestamp, newEvent, message, messageType, messageFlags, readTimestamp) "
                  "VALUES (:accountId, :threadId, :eventId, :senderId, :timestamp, :newEvent, :message, :messageType, :messageFlags, :readTimestamp)");
    query.bindValue(":accountId", event->accountId());
    query.bindValue(":threadId", event->threadId());
    query.bindValue(":eventId", event->eventId());
    query.bindValue(":senderId", event->senderId());
    query.bindValue(":timestamp", event->timestamp());
    query.bindValue(":newEvent", event->newEvent());
    query.bindValue(":message", event->message());
    query.bindValue(":messageType", event->messageType());
    query.bindValue(":messageFlags", (int) event->messageFlags());
    query.bindValue(":readTimestamp", event->readTimestamp());

    if (!query.exec()) {
        qCritical() << "Failed to save the text event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryWriter::removeTextEvent(const History::TextEventPtr &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    query.prepare("DELETE FROM text_events WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");
    query.bindValue(":accountId", event->accountId());
    query.bindValue(":threadId", event->threadId());
    query.bindValue(":eventId", event->eventId());

    if (!query.exec()) {
        qCritical() << "Failed to save the voice event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryWriter::writeVoiceEvent(const History::VoiceEventPtr &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    // FIXME: add support for checking if an event already exists

    query.prepare("INSERT INTO voice_events (accountId, threadId, eventId, senderId, timestamp, newEvent, duration, missed) "
                  "VALUES (:accountId, :threadId, :eventId, :senderId, :timestamp, :newEvent, :duration, :missed)");
    query.bindValue(":accountId", event->accountId());
    query.bindValue(":threadId", event->threadId());
    query.bindValue(":eventId", event->eventId());
    query.bindValue(":senderId", event->senderId());
    query.bindValue(":timestamp", event->timestamp());
    query.bindValue(":newEvent", event->newEvent());
    query.bindValue(":duration", QTime(0,0,0,0).secsTo(event->duration()));
    query.bindValue(":missed", event->missed());

    if (!query.exec()) {
        qCritical() << "Failed to save the voice event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryWriter::removeVoiceEvent(const History::VoiceEventPtr &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    query.prepare("DELETE FROM voice_events WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");
    query.bindValue(":accountId", event->accountId());
    query.bindValue(":threadId", event->threadId());
    query.bindValue(":eventId", event->eventId());

    if (!query.exec()) {
        qCritical() << "Failed to save the voice event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryWriter::beginBatchOperation()
{
    return SQLiteDatabase::instance()->beginTransation();
}

bool SQLiteHistoryWriter::endBatchOperation()
{
    return SQLiteDatabase::instance()->finishTransaction();
}

bool SQLiteHistoryWriter::rollbackBatchOperation()
{
    return SQLiteDatabase::instance()->rollbackTransaction();
}
