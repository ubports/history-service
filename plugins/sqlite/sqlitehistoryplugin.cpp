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

#include "sqlitehistoryplugin.h"
#include "phoneutils_p.h"
#include "sqlitedatabase.h"
#include "sqlitehistoryeventview.h"
#include "sqlitehistorythreadview.h"
#include <QDateTime>
#include <QDebug>
#include <QStringList>
#include <QSqlError>

SQLiteHistoryPlugin::SQLiteHistoryPlugin(QObject *parent) :
    QObject(parent)
{
    // just trigger the database creation or update
    SQLiteDatabase::instance();
}

// Reader
History::PluginThreadView *SQLiteHistoryPlugin::queryThreads(History::EventType type,
                                                             const History::Sort &sort,
                                                             const History::Filter &filter)
{
    return new SQLiteHistoryThreadView(this, type, sort, filter);
}

History::PluginEventView *SQLiteHistoryPlugin::queryEvents(History::EventType type,
                                                           const History::Sort &sort,
                                                           const History::Filter &filter)
{
    return new SQLiteHistoryEventView(this, type, sort, filter);
}

QVariantMap SQLiteHistoryPlugin::threadForParticipants(const QString &accountId,
                                                       History::EventType type,
                                                       const QStringList &participants,
                                                       History::MatchFlags matchFlags)
{
    if (participants.isEmpty()) {
        return QVariantMap();
    }

    QSqlQuery query(SQLiteDatabase::instance()->database());

    // select all the threads the first participant is listed in, and from that list
    // check if any of the threads has all the other participants listed
    // FIXME: find a better way to do this
    QString queryString("SELECT threadId FROM thread_participants WHERE %1 AND type=:type AND accountId=:accountId");

    // FIXME: for now we just compare differently when using MatchPhoneNumber
    if (matchFlags & History::MatchPhoneNumber) {
        queryString = queryString.arg("comparePhoneNumbers(participantId, :participantId)");
    } else {
        queryString = queryString.arg("participantId=:participantId");
    }
    query.prepare(queryString);
    query.bindValue(":participantId", participants[0]);
    query.bindValue(":type", type);
    query.bindValue(":accountId", accountId);
    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return QVariantMap();
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
            return QVariantMap();
        }

        QStringList threadParticipants;
        while (query.next()) {
            threadParticipants << query.value(0).toString();
        }

        // and now compare the lists
        bool found = true;
        Q_FOREACH(const QString &participant, participants) {
            if (matchFlags & History::MatchPhoneNumber) {
                // we need to iterate the list and call the phone number comparing function for
                // each participant from the given thread
                bool inList = false;
                Q_FOREACH(const QString &threadParticipant, threadParticipants) {
                    if (PhoneUtils::comparePhoneNumbers(threadParticipant, participant)) {
                        inList = true;
                        break;
                    }
                }
                if (!inList) {
                    found = false;
                    break;
                }
            } else if (!threadParticipants.contains(participant)) {
                found = false;
                break;
            }
        }

        if (found) {
            existingThread = threadId;
            break;
        }
    }

    QVariantMap thread;
    if (!existingThread.isNull()) {
        thread[History::FieldAccountId] = accountId;
        thread[History::FieldThreadId] = existingThread;
        thread[History::FieldType] = (int) type;
        thread[History::FieldParticipants] = participants;
    }

    return thread;
}

QList<QVariantMap> SQLiteHistoryPlugin::eventsForThread(const QVariantMap &thread)
{
    QList<QVariantMap> results;
    QString accountId = thread[History::FieldAccountId].toString();
    QString threadId = thread[History::FieldThreadId].toString();
    History::EventType type = (History::EventType) thread[History::FieldType].toInt();
    QString condition = QString("accountId=\"%1\" AND threadId=\"%2\"").arg(accountId, threadId);
    QString queryText = sqlQueryForEvents(type, condition, "");

    QSqlQuery query(SQLiteDatabase::instance()->database());
    if (!query.exec(queryText)) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return results;
    }

    results = parseEventResults(type, query);
    return results;
}

QVariantMap SQLiteHistoryPlugin::getSingleThread(History::EventType type, const QString &accountId, const QString &threadId)
{
    QVariantMap result;

    QString condition = QString("accountId=\"%1\" AND threadId=\"%2\"").arg(accountId, threadId);
    QString queryText = sqlQueryForThreads(type, condition, QString::null);
    queryText += " LIMIT 1";

    QSqlQuery query(SQLiteDatabase::instance()->database());
    if (!query.exec(queryText)) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return result;
    }

    QList<QVariantMap> results = parseThreadResults(type, query);
    query.clear();
    if (!results.isEmpty()) {
        result = results.first();
    }

    return result;
}

QVariantMap SQLiteHistoryPlugin::getSingleEvent(History::EventType type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    QVariantMap result;

    QString condition = QString("accountId=\"%1\" AND threadId=\"%2\" AND eventId=\"%3\"").arg(accountId, threadId, eventId);
    QString queryText = sqlQueryForEvents(type, condition, QString::null);
    queryText += " LIMIT 1";

    QSqlQuery query(SQLiteDatabase::instance()->database());
    if (!query.exec(queryText)) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return result;
    }

    QList<QVariantMap> results = parseEventResults(type, query);
    query.clear();
    if (!results.isEmpty()) {
        result = results.first();
    }

    return result;
}

// Writer
QVariantMap SQLiteHistoryPlugin::createThreadForParticipants(const QString &accountId, History::EventType type, const QStringList &participants)
{
    // WARNING: this function does NOT test to check if the thread is already created, you should check using HistoryReader::threadForParticipants()

    QVariantMap thread;

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
        return QVariantMap();
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
            return QVariantMap();
        }
    }

    // and finally create the thread
    thread[History::FieldAccountId] = accountId;
    thread[History::FieldThreadId] = threadId;
    thread[History::FieldType] = (int) type;
    thread[History::FieldParticipants] = participants;

    return thread;
}

bool SQLiteHistoryPlugin::removeThread(const QVariantMap &thread)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    query.prepare("DELETE FROM threads WHERE accountId=:accountId AND threadId=:threadId AND type=:type");
    query.bindValue(":accountId", thread[History::FieldAccountId]);
    query.bindValue(":threadId", thread[History::FieldThreadId]);
    query.bindValue(":type", thread[History::FieldType]);

    if (!query.exec()) {
        qCritical() << "Failed to remove the thread: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

History::EventWriteResult SQLiteHistoryPlugin::writeTextEvent(const QVariantMap &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    // check if the event exists
    QVariantMap existingEvent = getSingleEvent((History::EventType) event[History::FieldType].toInt(),
                                               event[History::FieldAccountId].toString(),
                                               event[History::FieldThreadId].toString(),
                                               event[History::FieldEventId].toString());

    History::EventWriteResult result;
    if (existingEvent.isEmpty()) {
        // create new
        query.prepare("INSERT INTO text_events (accountId, threadId, eventId, senderId, timestamp, newEvent, message, messageType, messageFlags, readTimestamp) "
                      "VALUES (:accountId, :threadId, :eventId, :senderId, :timestamp, :newEvent, :message, :messageType, :messageFlags, :readTimestamp)");
        result = History::EventWriteCreated;
    } else {
        // update existing event
        query.prepare("UPDATE text_events SET senderId=:senderId, timestamp=:timestamp, newEvent=:newEvent, message=:message, messageType=:messageType,"
                      "messageFlags=:messageFlags, readTimestamp=:readTimestamp WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");
        result = History::EventWriteModified;
    }

    query.bindValue(":accountId", event[History::FieldAccountId]);
    query.bindValue(":threadId", event[History::FieldThreadId]);
    query.bindValue(":eventId", event[History::FieldEventId]);
    query.bindValue(":senderId", event[History::FieldSenderId]);
    query.bindValue(":timestamp", QDateTime::fromString(event[History::FieldTimestamp].toString(), Qt::ISODate));
    query.bindValue(":newEvent", event[History::FieldNewEvent]);
    query.bindValue(":message", event[History::FieldMessage]);
    query.bindValue(":messageType", event[History::FieldMessageType]);
    query.bindValue(":messageFlags", event[History::FieldMessageFlags]);
    query.bindValue(":readTimestamp", QDateTime::fromString(event[History::FieldReadTimestamp].toString(), Qt::ISODate));

    if (!query.exec()) {
        qCritical() << "Failed to save the text event: Error:" << query.lastError() << query.lastQuery();
        return History::EventWriteError;
    }

    History::MessageType messageType = (History::MessageType) event[History::FieldMessageType].toInt();

    if (messageType == History::MessageTypeMultiParty) {
        // if the writing is an update, we need to remove the previous attachments
        if (result == History::EventWriteModified) {
            query.prepare("DELETE FROM text_event_attachments WHERE accountId=:accountId AND threadId=:threadId "
                          "AND eventId=:eventId");
            query.bindValue(":accountId", event[History::FieldAccountId]);
            query.bindValue(":threadId", event[History::FieldThreadId]);
            query.bindValue(":eventId", event[History::FieldEventId]);
            if (!query.exec()) {
                qCritical() << "Could not erase previous attachments. Error:" << query.lastError() << query.lastQuery();
                return History::EventWriteError;
            }
        }
        // save the attachments
        QList<QVariantMap> attachments = event[History::FieldAttachments].value<QList<QVariantMap> >();
        Q_FOREACH(const QVariantMap &attachment, attachments) {
            query.prepare("INSERT INTO text_event_attachments VALUES (:accountId, :threadId, :eventId, :attachmentId, :contentType, :filePath, :status)");
            query.bindValue(":accountId", attachment[History::FieldAccountId]);
            query.bindValue(":threadId", attachment[History::FieldThreadId]);
            query.bindValue(":eventId", attachment[History::FieldEventId]);
            query.bindValue(":attachmentId", attachment[History::FieldAttachmentId]);
            query.bindValue(":contentType", attachment[History::FieldContentType]);
            query.bindValue(":filePath", attachment[History::FieldFilePath]);
            query.bindValue(":status", attachment[History::FieldStatus]);
            if (!query.exec()) {
                qCritical() << "Failed to save attachment to database" << query.lastError() << attachment;
                return History::EventWriteError;
            }
        }
    }

    return result;
}

bool SQLiteHistoryPlugin::removeTextEvent(const QVariantMap &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    query.prepare("DELETE FROM text_events WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");
    query.bindValue(":accountId", event[History::FieldAccountId]);
    query.bindValue(":threadId", event[History::FieldThreadId]);
    query.bindValue(":eventId", event[History::FieldEventId]);

    if (!query.exec()) {
        qCritical() << "Failed to save the voice event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

History::EventWriteResult SQLiteHistoryPlugin::writeVoiceEvent(const QVariantMap &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    // check if the event exists
    QVariantMap existingEvent = getSingleEvent((History::EventType) event[History::FieldType].toInt(),
                                               event[History::FieldAccountId].toString(),
                                               event[History::FieldThreadId].toString(),
                                               event[History::FieldEventId].toString());

    History::EventWriteResult result;
    if (existingEvent.isEmpty()) {
        // create new
        query.prepare("INSERT INTO voice_events (accountId, threadId, eventId, senderId, timestamp, newEvent, duration, missed) "
                      "VALUES (:accountId, :threadId, :eventId, :senderId, :timestamp, :newEvent, :duration, :missed)");
        result = History::EventWriteCreated;
    } else {
        // update existing event
        query.prepare("UPDATE voice_events SET senderId=:senderId, timestamp=:timestamp, newEvent=:newEvent, duration=:duration, "
                      "missed=:missed WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");

        result = History::EventWriteModified;
    }

    query.bindValue(":accountId", event[History::FieldAccountId]);
    query.bindValue(":threadId", event[History::FieldThreadId]);
    query.bindValue(":eventId", event[History::FieldEventId]);
    query.bindValue(":senderId", event[History::FieldSenderId]);
    query.bindValue(":timestamp", event[History::FieldTimestamp]);
    query.bindValue(":newEvent", event[History::FieldNewEvent]);
    query.bindValue(":duration", event[History::FieldDuration]);
    query.bindValue(":missed", event[History::FieldMissed]);

    if (!query.exec()) {
        qCritical() << "Failed to save the voice event: Error:" << query.lastError() << query.lastQuery();
        result = History::EventWriteError;
    }

    return result;
}

bool SQLiteHistoryPlugin::removeVoiceEvent(const QVariantMap &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    query.prepare("DELETE FROM voice_events WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");
    query.bindValue(":accountId", event[History::FieldAccountId]);
    query.bindValue(":threadId", event[History::FieldThreadId]);
    query.bindValue(":eventId", event[History::FieldEventId]);

    if (!query.exec()) {
        qCritical() << "Failed to remove the voice event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryPlugin::beginBatchOperation()
{
    return SQLiteDatabase::instance()->beginTransation();
}

bool SQLiteHistoryPlugin::endBatchOperation()
{
    return SQLiteDatabase::instance()->finishTransaction();
}

bool SQLiteHistoryPlugin::rollbackBatchOperation()
{
    return SQLiteDatabase::instance()->rollbackTransaction();
}

QString SQLiteHistoryPlugin::sqlQueryForThreads(History::EventType type, const QString &condition, const QString &order)
{
    QString modifiedCondition = condition;
    if (!modifiedCondition.isEmpty()) {
        modifiedCondition.prepend(" AND ");
        // FIXME: the filters should be implemented in a better way
        modifiedCondition.replace("accountId=", "threads.accountId=");
        modifiedCondition.replace("threadId=", "threads.threadId=");
        modifiedCondition.replace("count=", "threads.count=");
        modifiedCondition.replace("unreadCount=", "threads.unreadCount=");
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

    QString queryText = QString("SELECT %1 FROM threads LEFT JOIN %2 ON threads.threadId=%2.threadId AND "
                         "threads.accountId=%2.accountId AND threads.lastEventId=%2.eventId WHERE threads.type=%3 %4 %5")
                         .arg(fields.join(", "), table, QString::number((int)type), modifiedCondition, order);
    return queryText;
}

QList<QVariantMap> SQLiteHistoryPlugin::parseThreadResults(History::EventType type, QSqlQuery &query)
{
    QList<QVariantMap> threads;
    while (query.next()) {
        QVariantMap thread;
        thread[History::FieldType] = (int) type;
        thread[History::FieldAccountId] = query.value(0);
        thread[History::FieldThreadId] = query.value(1);

        thread[History::FieldEventId] = query.value(2);
        thread[History::FieldCount] = query.value(3);
        thread[History::FieldUnreadCount] = query.value(4);
        thread[History::FieldParticipants] = query.value(5).toString().split("|,|");

        // the generic event fields
        thread[History::FieldSenderId] = query.value(6);
        thread[History::FieldTimestamp] = query.value(7);
        thread[History::FieldNewEvent] = query.value(8);

        // the next step is to get the last event
        switch (type) {
        case History::EventTypeText:
            thread[History::FieldMessage] = query.value(9);
            thread[History::FieldMessageType] = query.value(10);
            thread[History::FieldMessageFlags] = query.value(11);
            thread[History::FieldReadTimestamp] = query.value(12);
            break;
        case History::EventTypeVoice:
            thread[History::FieldMissed] = query.value(10);
            thread[History::FieldDuration] = QTime(0,0).addSecs(query.value(9).toInt());
            break;
        }
        threads << thread;
    }
    return threads;
}

QString SQLiteHistoryPlugin::sqlQueryForEvents(History::EventType type, const QString &condition, const QString &order)
{
    QString modifiedCondition = condition;
    if (!modifiedCondition.isEmpty()) {
        modifiedCondition.prepend(" WHERE ");
    }

    QString participantsField = "(SELECT group_concat(thread_participants.participantId,  \"|,|\") "
                                "FROM thread_participants WHERE thread_participants.accountId=%1.accountId "
                                "AND thread_participants.threadId=%1.threadId "
                                "AND thread_participants.type=%2 GROUP BY accountId,threadId,type) as participants";
    QString queryText;
    switch (type) {
    case History::EventTypeText:
        participantsField = participantsField.arg("text_events", QString::number(type));
        queryText = QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent, %1, "
                            "message, messageType, messageFlags, readTimestamp, subject FROM text_events %2 %3").arg(participantsField, modifiedCondition, order);
        break;
    case History::EventTypeVoice:
        participantsField = participantsField.arg("voice_events", QString::number(type));
        queryText = QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent, %1, "
                            "duration, missed FROM voice_events %2 %3").arg(participantsField, modifiedCondition, order);
        break;
    }

    return queryText;
}

QList<QVariantMap> SQLiteHistoryPlugin::parseEventResults(History::EventType type, QSqlQuery &query)
{
    QList<QVariantMap> events;
    while (query.next()) {
        QVariantMap event;
        History::MessageType messageType;
        QString accountId = query.value(0).toString();
        QString threadId = query.value(1).toString();
        QString eventId = query.value(2).toString();
        event[History::FieldType] = (int) type;
        event[History::FieldAccountId] = accountId;
        event[History::FieldThreadId] = threadId;
        event[History::FieldEventId] = eventId;
        event[History::FieldSenderId] = query.value(3);
        event[History::FieldTimestamp] = query.value(4);
        event[History::FieldNewEvent] = query.value(5);
        event[History::FieldParticipants] = query.value(6);


        switch (type) {
        case History::EventTypeText:
            messageType = (History::MessageType) query.value(7).toInt();
            if (messageType == History::MessageTypeMultiParty)  {
                QSqlQuery attachmentsQuery(SQLiteDatabase::instance()->database());
                attachmentsQuery.prepare("SELECT attachmentId, contentType, filePath, status FROM text_event_attachments "
                                    "WHERE accountId=:accountId and threadId=:threadId and eventId=:eventId");
                attachmentsQuery.bindValue(":accountId", accountId);
                attachmentsQuery.bindValue(":threadId", threadId);
                attachmentsQuery.bindValue(":eventId", eventId);
                if (!attachmentsQuery.exec()) {
                    qCritical() << "Error:" << attachmentsQuery.lastError() << attachmentsQuery.lastQuery();
                }

                QList<QVariantMap> attachments;
                while (attachmentsQuery.next()) {
                    QVariantMap attachment;
                    attachment[History::FieldAccountId] = accountId;
                    attachment[History::FieldThreadId] = threadId;
                    attachment[History::FieldEventId] = eventId;
                    attachment[History::FieldAttachmentId] = attachmentsQuery.value(0);
                    attachment[History::FieldContentType] = attachmentsQuery.value(1);
                    attachment[History::FieldFilePath] = attachmentsQuery.value(2);
                    attachment[History::FieldStatus] = attachmentsQuery.value(3);
                    attachments << attachment;

                }
                attachmentsQuery.clear();
                event[History::FieldAttachments] = QVariant::fromValue(attachments);
            }
            event[History::FieldMessage] = query.value(7);
            event[History::FieldMessageType] = query.value(8);
            event[History::FieldMessageFlags] = query.value(9);
            event[History::FieldReadTimestamp] = query.value(10);
            break;
        case History::EventTypeVoice:
            event[History::FieldDuration] = query.value(7).toInt();
            event[History::FieldMissed] = query.value(8);
            break;
        }

        events << event;
    }
    return events;
}
