/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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
#include "contactmatcher_p.h"
#include "utils_p.h"
#include <QDateTime>
#include <QDebug>
#include <QStringList>
#include <QSqlError>
#include <QDBusMetaType>
#include <QCryptographicHash>

static const QLatin1String timestampFormat("yyyy-MM-ddTHH:mm:ss.zzz");

QString generateThreadMapKey(const QString &accountId, const QString &threadId)
{
    return accountId + threadId;
}

QString generateThreadMapKey(const History::Thread &thread)
{
    return generateThreadMapKey(thread.accountId(), thread.threadId());
}

SQLiteHistoryPlugin::SQLiteHistoryPlugin(QObject *parent) :
    QObject(parent), mInitialised(false)
{
    // just trigger the database creation or update
    SQLiteDatabase::instance();
}

bool SQLiteHistoryPlugin::initialised()
{
    return mInitialised;
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
    Q_FOREACH (QVariantMap properties, threads) {
        // FIXME: it would be better to just use UTC all the way through the client,
        // but that requires a lot of changes
        // so instead we just convert to UTC here on the cache and convert back to local time
        // when returning
        QDateTime timestamp = QDateTime::fromString(properties[History::FieldTimestamp].toString(), Qt::ISODate);
        properties[History::FieldTimestamp] = timestamp.toUTC().toString(timestampFormat);

        // the same for readTimestamp
        timestamp = QDateTime::fromString(properties[History::FieldReadTimestamp].toString(), Qt::ISODate);
        properties[History::FieldReadTimestamp] = timestamp.toUTC().toString(timestampFormat);

        History::Thread thread = History::Thread::fromProperties(properties);
        const QString &threadKey = generateThreadMapKey(thread);

        if (thread.type() != History::EventTypeText) {
             continue;
        } else if (!History::Utils::shouldGroupThread(thread)) {
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
                if (!History::Utils::shouldGroupThread(groupedThread) || thread.chatType() != groupedThread.chatType()) {
                    continue;
                }
                found = History::Utils::compareNormalizedParticipants(thread.participants().identifiers(), groupedThread.participants().identifiers(), History::MatchPhoneNumber);
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
    QVariant leftValue = left[History::FieldLastEventTimestamp];
    QVariant rightValue = right[History::FieldLastEventTimestamp];

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
 
    if (thread.type() != History::EventTypeText || !History::Utils::shouldGroupThread(thread)) {
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

/**
 * @brief Parses the cached thread properties, change fields that might be necessary and return the data
 * @param thread the thread to extract properties from
 * @return the thread properties
 */
QVariantMap SQLiteHistoryPlugin::cachedThreadProperties(const History::Thread &thread) const
{
    QVariantMap properties = thread.properties();

    // FIXME: now we need to convert the timestamp back to local time
    // remove this once we change the flow to use UTC for everything
    QDateTime timestamp = QDateTime::fromString(properties[History::FieldTimestamp].toString(), Qt::ISODate);
    timestamp.setTimeSpec(Qt::UTC);
    properties[History::FieldTimestamp] = toLocalTimeString(timestamp);

    // and the readTimestamp too
    timestamp = QDateTime::fromString(properties[History::FieldReadTimestamp].toString(), Qt::ISODate);
    timestamp.setTimeSpec(Qt::UTC);
    properties[History::FieldReadTimestamp] = toLocalTimeString(timestamp);
    return properties;
}

/**
 * @brief Generates the cache containing contact data for all known participants.
 *
 * FIXME: this should probably be done outside of the plugin, but it requires a
 * refactory of \ref HistoryDaemon itself.
 */
void SQLiteHistoryPlugin::generateContactCache()
{
    QTime time;
    time.start();
    qDebug() << "---- HistoryService: start generating cached content";
    QSqlQuery query(SQLiteDatabase::instance()->database());
    if (!query.exec("SELECT DISTINCT accountId, normalizedId, alias, state FROM thread_participants")) {
        qWarning() << "Failed to generate contact cache:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        QString accountId = query.value(0).toString();
        QString participantId = query.value(1).toString();
        QString alias = query.value(2).toString();
        QVariantMap properties;
        if (!alias.isEmpty()) {
            properties[History::FieldAlias] = alias;
        }
        // we don't care about the results, as long as the contact data is present in the cache for
        // future usage.
        History::ContactMatcher::instance()->contactInfo(accountId, participantId, true, properties);
    }

    updateGroupedThreadsCache();

    qDebug() << "---- HistoryService: finished generating contact cache. elapsed time:" << time.elapsed() << "ms";

    mInitialised = true;
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

QVariantMap SQLiteHistoryPlugin::markThreadAsRead(const QVariantMap &thread)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    if (thread[History::FieldAccountId].toString().isEmpty() ||
           thread[History::FieldThreadId].toString().isEmpty()) {
        return QVariantMap();
    }

    // first check if the thread actually has anything to change
    query.prepare("SELECT unreadCount from threads WHERE accountId=:accountId AND threadId=:threadId AND type=:type");
    query.bindValue(":accountId", thread[History::FieldAccountId].toString());
    query.bindValue(":threadId", thread[History::FieldThreadId].toString());
    query.bindValue(":type", (uint)History::EventTypeText);
    if (!query.exec() || !query.next()) {
        qCritical() << "Failed to verify the unread messages of the thread. Error:" << query.lastError();
        return QVariantMap();
    }


    int unreadCount = query.value(0).toUInt();
    if (unreadCount == 0) {
        // no messages to ack, so no need to update anything
        return QVariantMap();
    }

    query.prepare("UPDATE text_events SET newEvent=:newEvent WHERE accountId=:accountId AND threadId=:threadId AND newEvent=1");
    query.bindValue(":accountId", thread[History::FieldAccountId].toString());
    query.bindValue(":threadId", thread[History::FieldThreadId].toString());
    query.bindValue(":newEvent", false);

    if (!query.exec()) {
        qCritical() << "Failed to mark thread as read: Error:" << query.lastError();
        return QVariantMap();
    }

    QVariantMap existingThread = getSingleThread((History::EventType) thread[History::FieldType].toInt(),
                                                 thread[History::FieldAccountId].toString(),
                                                 thread[History::FieldThreadId].toString(),
                                                 QVariantMap());
    if (!existingThread.isEmpty()) {
        addThreadsToCache(QList<QVariantMap>() << existingThread);
        return existingThread;
    }

    return QVariantMap();
}

QVariantMap SQLiteHistoryPlugin::threadForProperties(const QString &accountId,
                                                       History::EventType type,
                                                       const QVariantMap &properties,
                                                       History::MatchFlags matchFlags)
{
    if (properties.isEmpty()) {
        return QVariantMap();
    }

    History::ChatType chatType = (History::ChatType)properties[History::FieldChatType].toUInt();

    if (chatType == History::ChatTypeRoom) {
        QString threadId = properties[History::FieldThreadId].toString();
        if (threadId.isEmpty()) {
            return QVariantMap();
        }
        return getSingleThread(type, accountId, threadId);
    }

    History::Participants participants = History::Participants::fromVariant(properties[History::FieldParticipantIds]);
    // if chatType != Room, then we select the thread based on the participant list.
    return threadForParticipants(accountId, type, participants.identifiers(), matchFlags);
}

QString SQLiteHistoryPlugin::threadIdForProperties(const QString &accountId, History::EventType type, const QVariantMap &properties, History::MatchFlags matchFlags)
{
    if (properties.isEmpty()) {
        return QString::null;
    }

    // if chat type is room, just get the threadId directly
    History::ChatType chatType = (History::ChatType)properties[History::FieldChatType].toUInt();
    if (chatType == History::ChatTypeRoom) {
          QString threadId = properties[History::FieldThreadId].toString();
          return threadId;
    }

    // if chat type is anything else, fallback to returning the threadId from the participants list
    History::Participants participants = History::Participants::fromVariant(properties[History::FieldParticipantIds]);
    return threadForParticipants(accountId, type, participants.identifiers(), matchFlags)[History::FieldThreadId].toString();
}

QList<QVariantMap> SQLiteHistoryPlugin::participantsForThreads(const QList<QVariantMap> &threadIds)
{
    QList<QVariantMap> results;
    Q_FOREACH(const QVariantMap &thread, threadIds) {
        QString accountId = thread[History::FieldAccountId].toString();
        QString threadId = thread[History::FieldThreadId].toString();
        History::EventType type = (History::EventType)thread[History::FieldType].toUInt();
        QVariantMap result = thread;

        QSqlQuery query;
        query.prepare("SELECT normalizedId, alias, state, roles FROM thread_participants "
                      "WHERE accountId=:accountId AND threadId=:threadId AND type=:type");
        query.bindValue(":accountId", accountId);
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        QVariantList participants;
        if (!query.exec()) {
            qWarning() << "Failed to retrieve participants. Error:" << query.lastError().text() << query.lastQuery();
            results << result;
            continue;
        }

        while (query.next()) {
            QVariantMap participant;
            QString identifier = query.value(0).toString();
            participant[History::FieldIdentifier] = identifier;
            participant[History::FieldAlias] = query.value(1);
            participant[History::FieldParticipantState] = query.value(2);
            participant[History::FieldParticipantRoles] = query.value(3);
            participants << History::ContactMatcher::instance()->contactInfo(accountId, identifier, true, participant);
        }

        result[History::FieldParticipants] = participants;
        results << result;
    }
    return results;
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
    QString queryString("SELECT threadId FROM thread_participants WHERE %1 AND type=:type AND accountId=:accountId "
                        "AND (SELECT chatType FROM threads WHERE threads.threadId=thread_participants.threadId AND "
                        "      threads.type=thread_participants.type)!=:chatType");

    // FIXME: for now we just compare differently when using MatchPhoneNumber
    QString firstParticipant = participants.first();
    if (phoneCompare) {
        queryString = queryString.arg("compareNormalizedPhoneNumbers(normalizedId, :participantId)");
        firstParticipant = History::PhoneUtils::normalizePhoneNumber(firstParticipant);
    } else {
        queryString = queryString.arg("participantId=:participantId");
    }
    query.prepare(queryString);
    query.bindValue(":participantId", firstParticipant);
    query.bindValue(":type", type);
    query.bindValue(":accountId", accountId);
    // we don't want to accidentally return a chat room for a multi-recipient conversation
    query.bindValue(":chatType", (int)History::ChatTypeRoom);

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
            normalizedParticipants << History::PhoneUtils::normalizePhoneNumber(participant);
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
    if (properties.contains(History::FieldGroupingProperty)) {
        grouped = properties[History::FieldGroupingProperty].toString() == History::FieldParticipants;
    }
    if (grouped) {
        const QString &threadKey = generateThreadMapKey(accountId, threadId);
        // we have to find which conversation this thread belongs to
        if (mConversationsCacheKeys.contains(threadKey)) {
            // found the thread.
            // get the displayed thread now
            const History::Threads &groupedThreads = mConversationsCache[mConversationsCacheKeys[threadKey]];
            QVariantList finalGroupedThreads;
            Q_FOREACH(const History::Thread &displayedThread, groupedThreads) {
                QVariantMap properties = cachedThreadProperties(displayedThread);
                finalGroupedThreads << properties;
                if (generateThreadMapKey(displayedThread) == threadKey) {
                    result = properties;
                }
            }
            result[History::FieldGroupedThreads] = QVariant::fromValue(finalGroupedThreads);
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

bool SQLiteHistoryPlugin::updateRoomParticipants(const QString &accountId, const QString &threadId, History::EventType type, const QVariantList &participants)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());
    if (accountId.isEmpty() || threadId.isEmpty()) {
        return false;
    }

    SQLiteDatabase::instance()->beginTransation();
    QString deleteString("DELETE FROM thread_participants WHERE threadId=:threadId AND type=:type AND accountId=:accountId");
    query.prepare(deleteString);
    query.bindValue(":accountId", accountId);
    query.bindValue(":threadId", threadId);
    query.bindValue(":type", type);
    if (!query.exec()) {
        qCritical() << "Error removing old participants:" << query.lastError() << query.lastQuery();
        SQLiteDatabase::instance()->rollbackTransaction();
        return false;
    }

    // and insert the participants
    Q_FOREACH(const QVariant &participantVariant, participants) {
        QVariantMap participant = participantVariant.toMap();
        query.prepare("INSERT INTO thread_participants (accountId, threadId, type, participantId, normalizedId, alias, state, roles)"
                      "VALUES (:accountId, :threadId, :type, :participantId, :normalizedId, :alias, :state, :roles)");
        query.bindValue(":accountId", accountId);
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        query.bindValue(":participantId", participant["identifier"].toString());
        query.bindValue(":normalizedId", participant["identifier"].toString());
        query.bindValue(":alias", participant["alias"].toString());
        query.bindValue(":state", participant["state"].toUInt());
        query.bindValue(":roles", participant["roles"].toUInt());
        if (!query.exec()) {
            qCritical() << "Error:" << query.lastError() << query.lastQuery();
            SQLiteDatabase::instance()->rollbackTransaction();
            return false;
        }
    }

    if (!SQLiteDatabase::instance()->finishTransaction()) {
        qCritical() << "Failed to commit the transaction.";
        return false;
    }

    QVariantMap existingThread = getSingleThread(type,
                                                 accountId,
                                                 threadId,
                                                 QVariantMap());

    if (!existingThread.isEmpty()) {
        addThreadsToCache(QList<QVariantMap>() << existingThread);
    }

    return true;
}

bool SQLiteHistoryPlugin::updateRoomParticipantsRoles(const QString &accountId, const QString &threadId, History::EventType type, const QVariantMap &participantsRoles)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());
    if (accountId.isEmpty() || threadId.isEmpty()) {
        return false;
    }

    SQLiteDatabase::instance()->beginTransation();
    Q_FOREACH(const QString &participantId, participantsRoles.keys()) {
        query.prepare("UPDATE thread_participants SET roles=:roles WHERE accountId=:accountId AND threadId=:threadId AND type=:type AND participantId=:participantId");
        query.bindValue(":roles", participantsRoles.value(participantId).toUInt());
        query.bindValue(":accountId", accountId);
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        query.bindValue(":participantId", participantId);
        if (!query.exec()) {
            qCritical() << "Error:" << query.lastError() << query.lastQuery();
            SQLiteDatabase::instance()->rollbackTransaction();
            return false;
        }
    }

    if (!SQLiteDatabase::instance()->finishTransaction()) {
        qCritical() << "Failed to commit the transaction.";
        return false;
    }

    QVariantMap existingThread = getSingleThread(type,
                                                 accountId,
                                                 threadId,
                                                 QVariantMap());

    if (!existingThread.isEmpty()) {
        addThreadsToCache(QList<QVariantMap>() << existingThread);
    }

    return true;
}

bool SQLiteHistoryPlugin::updateRoomInfo(const QString &accountId, const QString &threadId, History::EventType type, const QVariantMap &properties, const QStringList &invalidated)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    if (threadId.isEmpty() || accountId.isEmpty()) {
        return false;
    }

    SQLiteDatabase::instance()->beginTransation();

    QDateTime creationTimestamp = QDateTime::fromTime_t(properties["CreationTimestamp"].toUInt());
    QDateTime timestamp = QDateTime::fromTime_t(properties["Timestamp"].toUInt());

    QVariantMap propertyMapping;
    propertyMapping["RoomName"] = "roomName";
    propertyMapping["Server"] = "server";
    propertyMapping["Creator"] = "creator";
    propertyMapping["CreationTimestamp"] = "creationTimestamp";
    propertyMapping["Anonymous"] = "anonymous";
    propertyMapping["InviteOnly"] = "inviteOnly";
    propertyMapping["Limit"] = "participantLimit";
    propertyMapping["Moderated"] = "moderated";
    propertyMapping["Title"] = "title";
    propertyMapping["Description"] = "description";
    propertyMapping["Persistent"] = "persistent";
    propertyMapping["Private"] = "private";
    propertyMapping["PasswordProtected"] = "passwordProtected";
    propertyMapping["Password"] = "password";
    propertyMapping["PasswordHint"] = "passwordHint";
    propertyMapping["CanUpdateConfiguration"] = "canUpdateConfiguration";
    propertyMapping["Subject"] = "subject";
    propertyMapping["Actor"] = "actor";
    propertyMapping["Timestamp"] = "timestamp";
    propertyMapping["Joined"] = "joined";
    propertyMapping["SelfRoles"] = "selfRoles";

    QStringList changedPropListValues;
    // populate sql query
    Q_FOREACH (const QString &key, properties.keys()) {
        if (propertyMapping.contains(key)) {
            QString prop = propertyMapping[key].toString();
            changedPropListValues << QString(prop+"=:"+ prop);
        }
    }
    if (changedPropListValues.isEmpty()) {
       return false;
    }

    query.prepare("UPDATE chat_room_info SET "+ changedPropListValues.join(", ")+" WHERE accountId=:accountId AND threadId=:threadId AND type=:type");
    query.bindValue(":accountId", accountId);
    query.bindValue(":threadId", threadId);
    query.bindValue(":type", (int) type);
    query.bindValue(":roomName", properties["RoomName"].toString());
    query.bindValue(":server", properties["Server"].toString());
    query.bindValue(":creator", properties["Creator"].toString());
    query.bindValue(":creationTimestamp", creationTimestamp.toUTC().toString(timestampFormat));
    query.bindValue(":anonymous", properties["Anonymous"].toBool());
    query.bindValue(":inviteOnly", properties["InviteOnly"].toBool());
    query.bindValue(":participantLimit", properties["Limit"].toInt());
    query.bindValue(":moderated", properties["Moderated"].toBool());
    query.bindValue(":title", properties["Title"].toString());
    query.bindValue(":description", properties["Description"].toString());
    query.bindValue(":persistent", properties["Persistent"].toBool());
    query.bindValue(":private", properties["Private"].toBool());
    query.bindValue(":passwordProtected", properties["PasswordProtected"].toBool());
    query.bindValue(":password", properties["Password"].toString());
    query.bindValue(":passwordHint", properties["PasswordHint"].toString());
    query.bindValue(":canUpdateConfiguration", properties["CanUpdateConfiguration"].toBool());
    query.bindValue(":subject", properties["Subject"].toString());
    query.bindValue(":actor", properties["Actor"].toString());
    query.bindValue(":timestamp", timestamp.toUTC().toString(timestampFormat));
    query.bindValue(":joined", properties["Joined"].toBool());
    query.bindValue(":selfRoles", properties["SelfRoles"].toInt());

    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        SQLiteDatabase::instance()->rollbackTransaction();
        return false;
    }

    if (!SQLiteDatabase::instance()->finishTransaction()) {
        qCritical() << "Failed to commit the transaction.";
        return false;
    }

    QVariantMap existingThread = getSingleThread(type,
                                                 accountId,
                                                 threadId,
                                                 QVariantMap());

    if (!existingThread.isEmpty()) {
        addThreadsToCache(QList<QVariantMap>() << existingThread);
    }

    return true;
}

QVariantMap SQLiteHistoryPlugin::createThreadForProperties(const QString &accountId, History::EventType type, const QVariantMap &properties)
{
    // WARNING: this function does NOT test to check if the thread is already created, you should check using HistoryReader::threadForParticipants()

    QVariantMap thread;
    History::Participants participants = History::Participants::fromVariant(properties[History::FieldParticipantIds]);

    // Create a new thread
    // FIXME: define what the threadId will be
    QString threadId;
    History::ChatType chatType = (History::ChatType)properties[History::FieldChatType].toInt();
    QVariantMap chatRoomInfo;

    SQLiteDatabase::instance()->beginTransation();

    if (chatType == History::ChatTypeRoom) {
        threadId = properties[History::FieldThreadId].toString();
        // we cannot save chat room without threadId
        if (accountId.isEmpty() || threadId.isEmpty()) {
            SQLiteDatabase::instance()->rollbackTransaction();
            return thread;
        }
        chatRoomInfo = properties[History::FieldChatRoomInfo].toMap();
        QSqlQuery query(SQLiteDatabase::instance()->database());

        QDateTime creationTimestamp = QDateTime::fromTime_t(chatRoomInfo["CreationTimestamp"].toUInt());
        QDateTime timestamp = QDateTime::fromTime_t(chatRoomInfo["Timestamp"].toUInt());

        query.prepare("INSERT INTO chat_room_info (accountId, threadId, type, roomName, server, creator, creationTimestamp, anonymous, inviteOnly, participantLimit, moderated, title, description, persistent, private, passwordProtected, password, passwordHint, canUpdateConfiguration, subject, actor, timestamp, joined, selfRoles) "
                      "VALUES (:accountId, :threadId, :type, :roomName, :server, :creator, :creationTimestamp, :anonymous, :inviteOnly, :participantLimit, :moderated, :title, :description, :persistent, :private, :passwordProtected, :password, :passwordHint, :canUpdateConfiguration, :subject, :actor, :timestamp, :joined, :selfRoles)");
        query.bindValue(":accountId", accountId);
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", (int) type);
        query.bindValue(":roomName", chatRoomInfo["RoomName"].toString());
        query.bindValue(":server", chatRoomInfo["Server"].toString());
        query.bindValue(":creator", chatRoomInfo["Creator"].toString());
        query.bindValue(":creationTimestamp", creationTimestamp.toUTC().toString(timestampFormat));
        query.bindValue(":anonymous", chatRoomInfo["Anonymous"].toBool());
        query.bindValue(":inviteOnly", chatRoomInfo["InviteOnly"].toBool());
        query.bindValue(":participantLimit", chatRoomInfo["Limit"].toInt());
        query.bindValue(":moderated", chatRoomInfo["Moderated"].toBool());
        query.bindValue(":title", chatRoomInfo["Title"].toString());
        query.bindValue(":description", chatRoomInfo["Description"].toString());
        query.bindValue(":persistent", chatRoomInfo["Persistent"].toBool());
        query.bindValue(":private", chatRoomInfo["Private"].toBool());
        query.bindValue(":passwordProtected", chatRoomInfo["PasswordProtected"].toBool());
        query.bindValue(":password", chatRoomInfo["Password"].toString());
        query.bindValue(":passwordHint", chatRoomInfo["PasswordHint"].toString());
        query.bindValue(":canUpdateConfiguration", chatRoomInfo["CanUpdateConfiguration"].toBool());
        query.bindValue(":subject", chatRoomInfo["Subject"].toString());
        query.bindValue(":actor", chatRoomInfo["Actor"].toString());
        query.bindValue(":timestamp", timestamp.toUTC().toString(timestampFormat));
        query.bindValue(":joined", chatRoomInfo["Joined"].toBool());
        query.bindValue(":selfRoles", chatRoomInfo["SelfRoles"].toInt());

        if (!query.exec()) {
            qCritical() << "Error:" << query.lastError() << query.lastQuery();
            SQLiteDatabase::instance()->rollbackTransaction();
            return QVariantMap();
        }
        for (QVariantMap::iterator iter = chatRoomInfo.begin(); iter != chatRoomInfo.end();) {
            if (!iter.value().isValid()) {
                iter = chatRoomInfo.erase(iter);
            } else {
                iter++;
            }
        }
        thread[History::FieldChatRoomInfo] = chatRoomInfo;
    } else if (chatType == History::ChatTypeContact) {
        threadId = participants.identifiers().join("%");
    } else {
        threadId = QString("broadcast:%1").arg(QString(QCryptographicHash::hash(participants.identifiers().join(";").toLocal8Bit(),QCryptographicHash::Md5).toHex()));;
    }

    QSqlQuery query(SQLiteDatabase::instance()->database());
    query.prepare("INSERT INTO threads (accountId, threadId, type, count, unreadCount, chatType, lastEventTimestamp)"
                  "VALUES (:accountId, :threadId, :type, :count, :unreadCount, :chatType, :lastEventTimestamp)");
    query.bindValue(":accountId", accountId);
    query.bindValue(":threadId", threadId);
    query.bindValue(":type", (int) type);
    query.bindValue(":count", 0);
    query.bindValue(":unreadCount", 0);
    query.bindValue(":chatType", (int) chatType);
    // make sure threads are created with an up-to-date timestamp
    query.bindValue(":lastEventTimestamp", QDateTime::currentDateTimeUtc().toString(timestampFormat));
    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        SQLiteDatabase::instance()->rollbackTransaction();
        return QVariantMap();
    }

    // and insert the participants
    Q_FOREACH(const History::Participant &participant, participants) {
        query.prepare("INSERT INTO thread_participants (accountId, threadId, type, participantId, normalizedId, alias, state, roles)"
                      "VALUES (:accountId, :threadId, :type, :participantId, :normalizedId, :alias, :state, :roles)");
        query.bindValue(":accountId", accountId);
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        query.bindValue(":participantId", participant.identifier());
        query.bindValue(":normalizedId", History::Utils::normalizeId(accountId, participant.identifier()));
        query.bindValue(":alias", participant.alias());
        query.bindValue(":state", participant.state());
        query.bindValue(":roles", participant.roles());
        if (!query.exec()) {
            qCritical() << "Error:" << query.lastError() << query.lastQuery();
            SQLiteDatabase::instance()->rollbackTransaction();
            return QVariantMap();
        }
    }

    if (!SQLiteDatabase::instance()->finishTransaction()) {
        qCritical() << "Failed to commit the transaction.";
        return QVariantMap();
    }

    // and finally create the thread
    thread[History::FieldAccountId] = accountId;
    thread[History::FieldThreadId] = threadId;
    thread[History::FieldType] = (int) type;
    QVariantList contactList;
    QVariantList contactInfo = History::ContactMatcher::instance()->contactInfo(accountId, participants.identifiers(), true);
    for (int i = 0; i < participants.count(); ++i) {
        QVariantMap map = contactInfo[i].toMap();
        History::Participant participant = participants[i];
        map["state"] = participant.state();
        map["roles"] = participant.roles();
        contactList << map;
    }
    thread[History::FieldParticipants] = contactList;
    thread[History::FieldCount] = 0;
    thread[History::FieldUnreadCount] = 0;
    thread[History::FieldChatType] = (int)chatType;

    addThreadsToCache(QList<QVariantMap>() << thread);

    return thread;
}

// Writer
QVariantMap SQLiteHistoryPlugin::createThreadForParticipants(const QString &accountId, History::EventType type, const QStringList &participants)
{
    QVariantMap properties;
    properties[History::FieldParticipantIds] = participants;
    properties[History::FieldChatType] = participants.size() != 1 ? History::ChatTypeNone : History::ChatTypeContact;
    return createThreadForProperties(accountId, type, properties);
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

    SQLiteDatabase::instance()->beginTransation();

    History::EventWriteResult result;
    if (existingEvent.isEmpty()) {
        // create new
        query.prepare("INSERT INTO text_events (accountId, threadId, eventId, senderId, timestamp, newEvent, message, messageType, messageStatus, readTimestamp, subject, informationType)"
                      "VALUES (:accountId, :threadId, :eventId, :senderId, :timestamp, :newEvent, :message, :messageType, :messageStatus, :readTimestamp, :subject, :informationType)");
        result = History::EventWriteCreated;
    } else {
        // update existing event
        query.prepare("UPDATE text_events SET senderId=:senderId, timestamp=:timestamp, newEvent=:newEvent, message=:message, messageType=:messageType, informationType=:informationType, "
                      "messageStatus=:messageStatus, readTimestamp=:readTimestamp, subject=:subject, informationType=:informationType WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");
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
    query.bindValue(":informationType", event[History::FieldInformationType].toInt());

    if (!query.exec()) {
        qCritical() << "Failed to save the text event: Error:" << query.lastError() << query.lastQuery();
        SQLiteDatabase::instance()->rollbackTransaction();
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
                SQLiteDatabase::instance()->rollbackTransaction();
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
                SQLiteDatabase::instance()->rollbackTransaction();
                return History::EventWriteError;
            }
        }
    }

    if (!SQLiteDatabase::instance()->finishTransaction()) {
        qCritical() << "Failed to commit transaction.";
        return History::EventWriteError;
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
        qCritical() << "Failed to remove the text event: Error:" << query.lastError() << query.lastQuery();
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
           << "threads.unreadCount"
           << "threads.lastEventTimestamp";

    QStringList extraFields;
    QString table;

    switch (type) {
    case History::EventTypeText:
        table = "text_events";
        extraFields << "text_events.message" << "text_events.messageType" << "text_events.messageStatus" << "text_events.readTimestamp" << "chatType" << "text_events.subject" << "text_events.informationType";
        break;
    case History::EventTypeVoice:
        table = "voice_events";
        extraFields << "voice_events.duration" << "voice_events.missed" << "voice_events.remoteParticipant";
        break;
    }

    fields << QString("%1.senderId").arg(table)
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
    QList<QVariantMap> threadsWithoutParticipants;
    QSqlQuery attachmentsQuery(SQLiteDatabase::instance()->database());
    QList<QVariantMap> attachments;
    bool grouped = false;
    if (properties.contains(History::FieldGroupingProperty)) {
        grouped = properties[History::FieldGroupingProperty].toString() == History::FieldParticipants;
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
            const QString &threadKey = generateThreadMapKey(accountId, threadId);
            if (mInitialised && type == History::EventTypeText && 
                !mConversationsCache.contains(threadKey)) {
                continue;
            }
            QVariantList groupedThreads;
            if (mConversationsCache.contains(threadKey)) {
                Q_FOREACH (const History::Thread &thread, mConversationsCache[threadKey]) {
                    groupedThreads << cachedThreadProperties(thread);
                }
            }
            thread[History::FieldGroupedThreads] = QVariant::fromValue(groupedThreads);
        }

        thread[History::FieldEventId] = query.value(2);
        thread[History::FieldCount] = query.value(3);
        thread[History::FieldUnreadCount] = query.value(4);

        // the generic event fields
        thread[History::FieldSenderId] = query.value(6);
        thread[History::FieldTimestamp] = toLocalTimeString(query.value(5).toDateTime());
        thread[History::FieldNewEvent] = query.value(7).toBool();

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
            thread[History::FieldMessage] = query.value(8);
            thread[History::FieldMessageType] = query.value(9);
            thread[History::FieldMessageStatus] = query.value(10);
            thread[History::FieldReadTimestamp] = toLocalTimeString(query.value(11).toDateTime());
            thread[History::FieldChatType] = query.value(12).toUInt();

            if (thread[History::FieldChatType].toInt() == History::ChatTypeRoom) {
                QVariantMap chatRoomInfo;
                QSqlQuery query1(SQLiteDatabase::instance()->database());

                query1.prepare("SELECT roomName, server, creator, creationTimestamp, anonymous, inviteOnly, participantLimit, moderated, title, description, persistent, private, passwordProtected, password, passwordHint, canUpdateConfiguration, subject, actor, timestamp, joined, selfRoles FROM chat_room_info WHERE accountId=:accountId AND threadId=:threadId AND type=:type LIMIT 1");
                query1.bindValue(":accountId", thread[History::FieldAccountId]);
                query1.bindValue(":threadId", thread[History::FieldThreadId]);
                query1.bindValue(":type", thread[History::FieldType].toInt());

                if (!query1.exec()) {
                    qCritical() << "Failed to get chat room info for thread: Error:" << query1.lastError() << query1.lastQuery();
                    break;
                }
                query1.next();

                if (query1.value(0).isValid())
                    chatRoomInfo["RoomName"] = query1.value(0);
                if (query1.value(1).isValid())
                    chatRoomInfo["Server"] = query1.value(1);
                if (query1.value(2).isValid())
                    chatRoomInfo["Creator"] = query1.value(2);
                if (query1.value(3).isValid())
                    chatRoomInfo["CreationTimestamp"] = toLocalTimeString(query1.value(3).toDateTime());
                if (query1.value(4).isValid())
                    chatRoomInfo["Anonymous"] = query1.value(4).toBool();
                if (query1.value(5).isValid())
                    chatRoomInfo["InviteOnly"] = query1.value(5).toBool();
                if (query1.value(6).isValid())
                    chatRoomInfo["Limit"] = query1.value(6).toInt();
                if (query1.value(7).isValid())
                    chatRoomInfo["Moderated"] = query1.value(7).toBool();
                if (query1.value(8).isValid())
                    chatRoomInfo["Title"] = query1.value(8);
                if (query1.value(9).isValid())
                    chatRoomInfo["Description"] = query1.value(9);
                if (query1.value(10).isValid())
                    chatRoomInfo["Persistent"] = query1.value(10).toBool();
                if (query1.value(11).isValid())
                    chatRoomInfo["Private"] = query1.value(11).toBool();
                if (query1.value(12).isValid())
                    chatRoomInfo["PasswordProtected"] = query1.value(12).toBool();
                if (query1.value(13).isValid())
                    chatRoomInfo["Password"] = query1.value(13);
                if (query1.value(14).isValid())
                    chatRoomInfo["PasswordHint"] = query1.value(14);
                if (query1.value(15).isValid())
                    chatRoomInfo["CanUpdateConfiguration"] = query1.value(15).toBool();
                if (query1.value(16).isValid())
                    chatRoomInfo["Subject"] = query1.value(16);
                if (query1.value(17).isValid())
                    chatRoomInfo["Actor"] = query1.value(17);
                if (query1.value(18).isValid())
                    chatRoomInfo["Timestamp"] = toLocalTimeString(query1.value(18).toDateTime());
                if (query1.value(19).isValid())
                    chatRoomInfo["Joined"] = query1.value(19).toBool();
                if (query1.value(20).isValid())
                    chatRoomInfo["SelfRoles"] = query1.value(20).toInt();

                thread[History::FieldChatRoomInfo] = chatRoomInfo;
            }
            if (!History::Utils::shouldIncludeParticipants(History::Thread::fromProperties(thread))) {
                thread.remove(History::FieldParticipants);
                threadsWithoutParticipants << thread;
            } else {
                threads << thread;
            }
            break;
        case History::EventTypeVoice:
            thread[History::FieldMissed] = query.value(9);
            thread[History::FieldDuration] = query.value(8);
            thread[History::FieldRemoteParticipant] = History::ContactMatcher::instance()->contactInfo(accountId, query.value(10).toString(), true);
            threads << thread;
            break;
        }
    }

    // get the participants
    threads = participantsForThreads(threads);

    // and append the threads with no participants
    threads << threadsWithoutParticipants;

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
        // for text events we don't need the participants at all
        participantsField = "\"\" as participants";
        queryText = QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent, %1, "
                            "message, messageType, messageStatus, readTimestamp, subject, informationType FROM text_events %2 %3").arg(participantsField, modifiedCondition, order);
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

        // ignore events that don't have a threadId or an eventId
        if (threadId.trimmed().isEmpty() || eventId.trimmed().isEmpty()) {
            continue;
        }

        event[History::FieldType] = (int) type;
        event[History::FieldAccountId] = accountId;
        event[History::FieldThreadId] = threadId;
        event[History::FieldEventId] = eventId;
        event[History::FieldSenderId] = query.value(3);
        event[History::FieldTimestamp] = toLocalTimeString(query.value(4).toDateTime());
        event[History::FieldNewEvent] = query.value(5).toBool();
        if (type != History::EventTypeText) {
            QStringList participants = query.value(6).toString().split("|,|");
            event[History::FieldParticipants] = History::ContactMatcher::instance()->contactInfo(accountId, participants, true);
        }

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
            if (!query.value(11).toString().isEmpty()) {
                event[History::FieldSubject] = query.value(11).toString();
            }
            event[History::FieldInformationType] = query.value(12).toInt();
            break;
        case History::EventTypeVoice:
            event[History::FieldDuration] = query.value(7).toInt();
            event[History::FieldMissed] = query.value(8);
            event[History::FieldRemoteParticipant] = query.value(9).toString();
            break;
        }

        events << event;
    }
    return events;
}

QString SQLiteHistoryPlugin::toLocalTimeString(const QDateTime &timestamp)
{
    return QDateTime(timestamp.date(), timestamp.time(), Qt::UTC).toLocalTime().toString(timestampFormat);
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
        } else if (filter.matchFlags() & History::MatchNotEquals) {
            result = QString("%1!=%2").arg(propertyName, bindId);
            bindValues[bindId] = filterValue;
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
