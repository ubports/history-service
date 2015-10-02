/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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
#include "utils_p.h"
#include "sqlitedatabase.h"
#include "sqlitehistoryeventview.h"
#include "sqlitehistorythreadview.h"
#include "intersectionfilter.h"
#include "unionfilter.h"
#include "thread.h"
#include <QDateTime>
#include <QDebug>
#include <QStringList>
#include <QSqlError>
#include <QDBusMetaType>

Q_DECLARE_METATYPE(History::Thread)
Q_DECLARE_METATYPE(History::Threads)

QString generateThreadMapKey(const History::Thread &thread)
{
    return thread.accountId() + thread.threadId();
}

SQLiteHistoryPlugin::SQLiteHistoryPlugin(QObject *parent) :
    QObject(parent), mInitialised(false)
{
    qRegisterMetaType<History::Thread>();
    qRegisterMetaType<History::Threads>();
    // just trigger the database creation or update
    SQLiteDatabase::instance();

    updateGroupedThreadsCache();

    mInitialised = true;
}

void SQLiteHistoryPlugin::updateGroupedThreadsCache()
{
    History::PluginThreadView *view = queryThreads(History::EventTypeText, History::Sort("timestamp", Qt::DescendingOrder), History::Filter());
    QList<QVariantMap> threads;
    while (view->IsValid()) {
        QList<QVariantMap> page = view->NextPage();
        if (page.size() > 0) {
            threads += page;
        } else {
            break;
        }
    }
    addThreadsToCache(threads);
}

void SQLiteHistoryPlugin::addThreadsToCache(const QList<QVariantMap> &threads)
{
    Q_FOREACH (const QVariantMap &properties, threads) {
        History::Thread thread = History::Thread::fromProperties(properties);
        const QString &threadKey = generateThreadMapKey(thread);

        if (thread.type() != History::EventTypeText) {
             continue;
        } else if (!History::Utils::shouldGroupAccount(thread.accountId())) {
            // never group non phone accounts
            mConversationsCache[threadKey] = History::Threads() << thread;
            mConversationsCacheKeys[threadKey] = threadKey;
            continue;
        }
        // find conversation grouping this thread
        if (mConversationsCacheKeys.contains(threadKey)) {
            QString conversationKey = mConversationsCacheKeys[threadKey];
            History::Threads groupedThreads = mConversationsCache[conversationKey];
            Q_FOREACH(const History::Thread &groupedThread, groupedThreads) {
                mConversationsCacheKeys.remove(generateThreadMapKey(groupedThread));
            }
            groupedThreads.removeAll(thread);
            groupedThreads.append(thread);
            mConversationsCache[conversationKey] = groupedThreads;
            mConversationsCacheKeys.remove(threadKey);
            updateDisplayedThread(conversationKey);
            continue;
        }
        // if not found, we have to iterate the list and compare phone numbers
        bool found = false;
        QMap<QString, History::Threads>::iterator it = mConversationsCache.begin();
        while (it != mConversationsCache.end()) {
            const QString &conversationKey = it.key();
            History::Threads groupedThreads = it.value();
            Q_FOREACH(const History::Thread &groupedThread, groupedThreads) {
                found = History::Utils::compareNormalizedParticipants(thread.participants(), groupedThread.participants(), History::MatchPhoneNumber);
                if (found) {
                    Q_FOREACH(const History::Thread &groupedThread, groupedThreads) {
                        mConversationsCacheKeys.remove(generateThreadMapKey(groupedThread));
                    }
                    mConversationsCache[conversationKey] += thread;
                    updateDisplayedThread(conversationKey);
                    break;
                }
            }
            if (found) {
                break;
            }
            it++;
        }
        if (!found) {
            mConversationsCache[threadKey] = History::Threads() << thread;
            mConversationsCacheKeys[threadKey] = threadKey;
        }
    }
}

bool SQLiteHistoryPlugin::lessThan(const QVariantMap &left, const QVariantMap &right) const
{
    QVariant leftValue = left["lastEventTimestamp"];
    QVariant rightValue = right["lastEventTimestamp"];

    return leftValue < rightValue;
}

void SQLiteHistoryPlugin::updateDisplayedThread(const QString &displayedThreadKey)
{
    History::Threads threads = mConversationsCache[displayedThreadKey];
    History::Thread displayedThread = threads.first();
    QVariantMap displayedProperties = displayedThread.properties();
    Q_FOREACH(const History::Thread &other, threads) {
        if (lessThan(displayedProperties, other.properties())) {
            displayedThread = other;
            displayedProperties = displayedThread.properties();
        }
    }

    QString newDisplayedThreadKey = generateThreadMapKey(displayedThread);
    mConversationsCache.remove(displayedThreadKey);
    mConversationsCache[newDisplayedThreadKey] = threads;

    // update reverse threadId -> conversationId map
    Q_FOREACH(const History::Thread &groupedThread, threads) {
        mConversationsCacheKeys[generateThreadMapKey(groupedThread)] = newDisplayedThreadKey;
    }
}

void SQLiteHistoryPlugin::removeThreadFromCache(const QVariantMap &properties)
{
    History::Thread thread = History::Thread::fromProperties(properties);
    QString threadKey = generateThreadMapKey(thread);
 
    if (thread.type() != History::EventTypeText || !History::Utils::shouldGroupAccount(thread.accountId())) {
        mConversationsCache.remove(threadKey);
        mConversationsCacheKeys.remove(threadKey);
        return;
    }
 
    // check if this is a main key first
    if (mConversationsCache.contains(threadKey)) {
        // Remove itself from the list and promote the next grouped thread if any
        History::Threads threads = mConversationsCache[threadKey];
        threads.removeAll(thread);
        mConversationsCache.remove(threadKey);
        mConversationsCacheKeys.remove(threadKey);
        // remove all threads from reverse map. they will be readded
        // in updateDisplayedThread() if needed
        Q_FOREACH (const History::Thread &thread, threads) {
            mConversationsCacheKeys.remove(generateThreadMapKey(thread));
        }
        if (!threads.isEmpty()) {
            threadKey = generateThreadMapKey(threads.first());
            mConversationsCache[threadKey] = threads;
            updateDisplayedThread(threadKey);
        }
    } else {
        // check if it belongs to an existing grouped thread;
        QMap<QString, History::Threads>::iterator it = mConversationsCache.begin();
        while (it != mConversationsCache.end()) {
            const QString &threadKey = it.key();
            History::Threads threads = it.value();
            int pos = threads.indexOf(thread);
            if (pos != -1) {
                const QString &threadKey = generateThreadMapKey(thread);
                mConversationsCache.remove(threadKey);
                mConversationsCacheKeys.remove(threadKey);
                if (threads.size() == 1) {
                   return;
                } else {
                    threads.removeAll(thread);
                    const QString &newThreadKey = generateThreadMapKey(threads.first());
                    mConversationsCache[newThreadKey] = threads;
                    updateDisplayedThread(newThreadKey);
                    return;
                }
            }
            it++;
        }
    }
}

// Reader
History::PluginThreadView *SQLiteHistoryPlugin::queryThreads(History::EventType type,
                                                             const History::Sort &sort,
                                                             const History::Filter &filter,
                                                             const QVariantMap &properties)
{
    return new SQLiteHistoryThreadView(this, type, sort, filter, properties);
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

    bool phoneCompare = (matchFlags & History::MatchPhoneNumber);
    QSqlQuery query(SQLiteDatabase::instance()->database());

    // select all the threads the first participant is listed in, and from that list
    // check if any of the threads has all the other participants listed
    // FIXME: find a better way to do this
    QString queryString("SELECT threadId FROM thread_participants WHERE %1 AND type=:type AND accountId=:accountId");

    // FIXME: for now we just compare differently when using MatchPhoneNumber
    QString firstParticipant = participants.first();
    if (phoneCompare) {
        queryString = queryString.arg("compareNormalizedPhoneNumbers(normalizedId, :participantId)");
        firstParticipant = PhoneUtils::normalizePhoneNumber(firstParticipant);
    } else {
        queryString = queryString.arg("participantId=:participantId");
    }
    query.prepare(queryString);
    query.bindValue(":participantId", firstParticipant);
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
    QStringList normalizedParticipants;
    if (phoneCompare) {
        Q_FOREACH(const QString &participant, participants) {
            normalizedParticipants << PhoneUtils::normalizePhoneNumber(participant);
        }
    } else {
        normalizedParticipants = participants;
    }

    // now for each threadId, check if all the other participants are listed
    Q_FOREACH(const QString &threadId, threadIds) {
        queryString = "SELECT %1 FROM thread_participants WHERE "
                      "threadId=:threadId AND type=:type AND accountId=:accountId";
        query.prepare(queryString.arg(phoneCompare ? "normalizedId" : "participantId"));
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

        // we can't use query.size() as it always return -1
        if (threadParticipants.count() != normalizedParticipants.count()) {
            continue;
        }

        bool found = History::Utils::compareNormalizedParticipants(threadParticipants, normalizedParticipants, matchFlags);
        if (found) {
            existingThread = threadId;
            break;
        }
    }

    return getSingleThread(type, accountId, existingThread);
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

QVariantMap SQLiteHistoryPlugin::getSingleThread(History::EventType type, const QString &accountId, const QString &threadId, const QVariantMap &properties)
{
    QVariantMap result;
    bool grouped = false;
    if (accountId.isEmpty() || threadId.isEmpty()) {
        return result;
    }
    if (properties.contains("groupingProperty")) {
        grouped = properties["groupingProperty"].toString() == History::FieldParticipants;
    }
    if (grouped) {
        const QString &threadKey = accountId+threadId;
        // we have to find which conversation this thread belongs to
        if (mConversationsCacheKeys.contains(threadKey)) {
            // found the thread.
            // get the displayed thread now
            const History::Threads &groupedThreads = mConversationsCache[mConversationsCacheKeys[threadKey]];
            QVariantList finalGroupedThreads;
            Q_FOREACH(const History::Thread &displayedThread, groupedThreads) {
                finalGroupedThreads << displayedThread.properties();
                if (generateThreadMapKey(displayedThread) == threadKey) {
                    result = displayedThread.properties();
                }
            }
            result[History::FieldGroupedThreads] = finalGroupedThreads;
            return result;
        }
        return result;
    }

    QString condition = QString("accountId=\"%1\" AND threadId=\"%2\"").arg(accountId, threadId);
    QString queryText = sqlQueryForThreads(type, condition, QString::null);
    queryText += " LIMIT 1";

    QSqlQuery query(SQLiteDatabase::instance()->database());
    if (!query.exec(queryText)) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return result;
    }

    QList<QVariantMap> results = parseThreadResults(type, query, properties);
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
    query.bindValue(":type", (int) type);
    query.bindValue(":count", 0);
    query.bindValue(":unreadCount", 0);
    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return QVariantMap();
    }

    // and insert the participants
    Q_FOREACH(const QString &participant, participants) {
        query.prepare("INSERT INTO thread_participants (accountId, threadId, type, participantId, normalizedId)"
                      "VALUES (:accountId, :threadId, :type, :participantId, normalizeId(:accountId, :participantId))");
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
    thread[History::FieldCount] = 0;
    thread[History::FieldUnreadCount] = 0;

    addThreadsToCache(QList<QVariantMap>() << thread);

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

    removeThreadFromCache(thread);

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
        query.prepare("INSERT INTO text_events (accountId, threadId, eventId, senderId, timestamp, newEvent, message, messageType, messageStatus, readTimestamp, subject) "
                      "VALUES (:accountId, :threadId, :eventId, :senderId, :timestamp, :newEvent, :message, :messageType, :messageStatus, :readTimestamp, :subject)");
        result = History::EventWriteCreated;
    } else {
        // update existing event
        query.prepare("UPDATE text_events SET senderId=:senderId, timestamp=:timestamp, newEvent=:newEvent, message=:message, messageType=:messageType,"
                      "messageStatus=:messageStatus, readTimestamp=:readTimestamp, subject=:subject WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");
        result = History::EventWriteModified;
    }

    query.bindValue(":accountId", event[History::FieldAccountId]);
    query.bindValue(":threadId", event[History::FieldThreadId]);
    query.bindValue(":eventId", event[History::FieldEventId]);
    query.bindValue(":senderId", event[History::FieldSenderId]);
    query.bindValue(":timestamp", event[History::FieldTimestamp].toDateTime().toUTC());
    query.bindValue(":newEvent", event[History::FieldNewEvent]);
    query.bindValue(":message", event[History::FieldMessage]);
    query.bindValue(":messageType", event[History::FieldMessageType]);
    query.bindValue(":messageStatus", event[History::FieldMessageStatus]);
    query.bindValue(":readTimestamp", event[History::FieldReadTimestamp].toDateTime().toUTC());
    query.bindValue(":subject", event[History::FieldSubject].toString());

    if (!query.exec()) {
        qCritical() << "Failed to save the text event: Error:" << query.lastError() << query.lastQuery();
        return History::EventWriteError;
    }

    History::MessageType messageType = (History::MessageType) event[History::FieldMessageType].toInt();

    if (messageType == History::MessageTypeMultiPart) {
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
        QList<QVariantMap> attachments = qdbus_cast<QList<QVariantMap> >(event[History::FieldAttachments]);
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

    if (result == History::EventWriteModified || result == History::EventWriteCreated) {
        QVariantMap existingThread = getSingleThread((History::EventType) event[History::FieldType].toInt(),
                                                     event[History::FieldAccountId].toString(),
                                                     event[History::FieldThreadId].toString(),
                                                     QVariantMap());
        addThreadsToCache(QList<QVariantMap>() << existingThread);

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

    QVariantMap existingThread = getSingleThread((History::EventType) event[History::FieldType].toInt(),
                                                 event[History::FieldAccountId].toString(),
                                                 event[History::FieldThreadId].toString(),
                                                 QVariantMap());
    if (!existingThread.isEmpty()) {
        addThreadsToCache(QList<QVariantMap>() << existingThread);
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
        query.prepare("INSERT INTO voice_events (accountId, threadId, eventId, senderId, timestamp, newEvent, duration, missed, remoteParticipant) "
                      "VALUES (:accountId, :threadId, :eventId, :senderId, :timestamp, :newEvent, :duration, :missed, :remoteParticipant)");
        result = History::EventWriteCreated;
    } else {
        // update existing event
        query.prepare("UPDATE voice_events SET senderId=:senderId, timestamp=:timestamp, newEvent=:newEvent, duration=:duration, "
                      "missed=:missed, remoteParticipant=:remoteParticipant "
                      "WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");

        result = History::EventWriteModified;
    }

    query.bindValue(":accountId", event[History::FieldAccountId]);
    query.bindValue(":threadId", event[History::FieldThreadId]);
    query.bindValue(":eventId", event[History::FieldEventId]);
    query.bindValue(":senderId", event[History::FieldSenderId]);
    query.bindValue(":timestamp", event[History::FieldTimestamp].toDateTime().toUTC());
    query.bindValue(":newEvent", event[History::FieldNewEvent]);
    query.bindValue(":duration", event[History::FieldDuration]);
    query.bindValue(":missed", event[History::FieldMissed]);
    query.bindValue(":remoteParticipant", event[History::FieldRemoteParticipant]);

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

    QString modifiedOrder = order;
    if (!modifiedOrder.isEmpty()) {
        modifiedOrder.replace(" accountId", " threads.accountId");
        modifiedOrder.replace(" threadId", " threads.threadId");
        modifiedOrder.replace(" count", " threads.count");
        modifiedOrder.replace(" unreadCount", " threads.unreadCount");
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
        extraFields << "text_events.message" << "text_events.messageType" << "text_events.messageStatus" << "text_events.readTimestamp";
        break;
    case History::EventTypeVoice:
        table = "voice_events";
        extraFields << "voice_events.duration" << "voice_events.missed" << "voice_events.remoteParticipant";
        break;
    }

    fields << QString("%1.senderId").arg(table)
           << QString("%1.timestamp").arg(table)
           << QString("%1.newEvent").arg(table);
    fields << extraFields;

    QString queryText = QString("SELECT %1 FROM threads LEFT JOIN %2 ON threads.threadId=%2.threadId AND "
                         "threads.accountId=%2.accountId AND threads.lastEventId=%2.eventId WHERE threads.type=%3 %4 %5")
                         .arg(fields.join(", "), table, QString::number((int)type), modifiedCondition, modifiedOrder);
    return queryText;
}

QList<QVariantMap> SQLiteHistoryPlugin::parseThreadResults(History::EventType type, QSqlQuery &query, const QVariantMap &properties)
{
    QList<QVariantMap> threads;
    QSqlQuery attachmentsQuery(SQLiteDatabase::instance()->database());
    QList<QVariantMap> attachments;
    bool grouped = false;
    if (properties.contains("groupingProperty")) {
        grouped = properties["groupingProperty"].toBool();
    }
    while (query.next()) {
        QVariantMap thread;
        QString accountId = query.value(0).toString();                   
        QString threadId = query.value(1).toString();
        if (threadId.trimmed().isEmpty()) {
            continue;
        }
        thread[History::FieldType] = (int) type;
        thread[History::FieldAccountId] = accountId;
        thread[History::FieldThreadId] = threadId;
        if (grouped) {
            const QString &threadKey = accountId+threadId;
            if (mInitialised && type == History::EventTypeText && 
                !mConversationsCache.contains(threadKey)) {
                continue;
            }
            QList<QVariantMap> groupedThreads;
            if (mConversationsCache.contains(threadKey)) {
                Q_FOREACH (const History::Thread &thread, mConversationsCache[threadKey]) {
                    groupedThreads << thread.properties();
                }
            }
	    thread[History::FieldGroupedThreads] = QVariant::fromValue(groupedThreads);
        }

        thread[History::FieldEventId] = query.value(2);
        thread[History::FieldCount] = query.value(3);
        thread[History::FieldUnreadCount] = query.value(4);
        thread[History::FieldParticipants] = query.value(5).toString().split("|,|");

        // the generic event fields
        thread[History::FieldSenderId] = query.value(6);
        thread[History::FieldTimestamp] = toLocalTimeString(query.value(7).toDateTime());
        thread[History::FieldNewEvent] = query.value(8);

        // the next step is to get the last event
        switch (type) {
        case History::EventTypeText:
            attachmentsQuery.prepare("SELECT attachmentId, contentType, filePath, status FROM text_event_attachments "
                                "WHERE accountId=:accountId and threadId=:threadId and eventId=:eventId");
            attachmentsQuery.bindValue(":accountId", query.value(0));
            attachmentsQuery.bindValue(":threadId", query.value(1));
            attachmentsQuery.bindValue(":eventId", query.value(2));
            if (!attachmentsQuery.exec()) {
                qCritical() << "Error:" << attachmentsQuery.lastError() << attachmentsQuery.lastQuery();
            }

            while (attachmentsQuery.next()) {
                QVariantMap attachment;
                attachment[History::FieldAccountId] = query.value(0);
                attachment[History::FieldThreadId] = query.value(1);
                attachment[History::FieldEventId] = query.value(2);
                attachment[History::FieldAttachmentId] = attachmentsQuery.value(0);
                attachment[History::FieldContentType] = attachmentsQuery.value(1);
                attachment[History::FieldFilePath] = attachmentsQuery.value(2);
                attachment[History::FieldStatus] = attachmentsQuery.value(3);
                attachments << attachment;

            }
            attachmentsQuery.clear();
            if (attachments.size() > 0) {
                thread[History::FieldAttachments] = QVariant::fromValue(attachments);
                attachments.clear();
            }
            thread[History::FieldMessage] = query.value(9);
            thread[History::FieldMessageType] = query.value(10);
            thread[History::FieldMessageStatus] = query.value(11);
            thread[History::FieldReadTimestamp] = toLocalTimeString(query.value(12).toDateTime());
            break;
        case History::EventTypeVoice:
            thread[History::FieldMissed] = query.value(10);
            thread[History::FieldDuration] = query.value(9);
            thread[History::FieldRemoteParticipant] = query.value(11);
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
                            "message, messageType, messageStatus, readTimestamp, subject FROM text_events %2 %3").arg(participantsField, modifiedCondition, order);
        break;
    case History::EventTypeVoice:
        participantsField = participantsField.arg("voice_events", QString::number(type));
        queryText = QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent, %1, "
                            "duration, missed, remoteParticipant FROM voice_events %2 %3").arg(participantsField, modifiedCondition, order);
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
        event[History::FieldTimestamp] = toLocalTimeString(query.value(4).toDateTime());
        event[History::FieldNewEvent] = query.value(5);
        event[History::FieldParticipants] = query.value(6).toString().split("|,|");


        switch (type) {
        case History::EventTypeText:
            messageType = (History::MessageType) query.value(8).toInt();
            if (messageType == History::MessageTypeMultiPart)  {
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
            event[History::FieldMessageStatus] = query.value(9);
            event[History::FieldReadTimestamp] = toLocalTimeString(query.value(10).toDateTime());
            break;
        case History::EventTypeVoice:
            event[History::FieldDuration] = query.value(7).toInt();
            event[History::FieldMissed] = query.value(8);
            event[History::FieldRemoteParticipant] = query.value(9);
            break;
        }

        events << event;
    }
    return events;
}

QString SQLiteHistoryPlugin::toLocalTimeString(const QDateTime &timestamp)
{
    return QDateTime(timestamp.date(), timestamp.time(), Qt::UTC).toLocalTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
}

QString SQLiteHistoryPlugin::filterToString(const History::Filter &filter, QVariantMap &bindValues, const QString &propertyPrefix) const
{
    QString result;
    History::Filters filters;
    QString linking;
    QString value;
    int count;
    QString filterProperty = filter.filterProperty();
    QVariant filterValue = filter.filterValue();

    switch (filter.type()) {
    case History::FilterTypeIntersection:
        filters = History::IntersectionFilter(filter).filters();
        linking = " AND ";
    case History::FilterTypeUnion:
        if (filter.type() == History::FilterTypeUnion) {
            filters = History::UnionFilter(filter).filters();
            linking = " OR ";
        }

        if (filters.isEmpty()) {
            break;
        }

        result = "( ";
        count = filters.count();
        for (int i = 0; i < count; ++i) {
            // run recursively through the inner filters
            result += QString("(%1)").arg(filterToString(filters[i], bindValues, propertyPrefix));
            if (i != count-1) {
                result += linking;
            }
        }
        result += " )";
        break;
    default:
        if (filterProperty.isEmpty() || filterValue.isNull()) {
            break;
        }

        QString bindId = QString(":filterValue%1").arg(bindValues.count());

        QString propertyName = propertyPrefix.isNull() ? filterProperty : QString("%1.%2").arg(propertyPrefix, filterProperty);
        // FIXME: need to check for other match flags and multiple match flags
        if (filter.matchFlags() & History::MatchContains) {
            // FIXME: maybe we should use QString("%1 LIKE '\%'||%2'\%'").arg(bindId) ?? needs more time for investigating
            result = QString("%1 LIKE '\%%2\%' ESCAPE '\\'").arg(propertyName, escapeFilterValue(filterValue.toString()));
        } else {
            result = QString("%1=%2").arg(propertyName, bindId);
            bindValues[bindId] = filterValue;
        }
    }

    return result;
}

QString SQLiteHistoryPlugin::escapeFilterValue(const QString &value) const
{
    QString escaped = value;
    escaped.replace("\\", "\\\\")
           .replace("'", "''")
           .replace("%", "\\%")
           .replace("_", "\\_");
    return escaped;
}
