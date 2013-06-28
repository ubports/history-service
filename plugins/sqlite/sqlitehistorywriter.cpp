#include "sqlitehistorywriter.h"
#include "sqlitedatabase.h"
#include <HistoryItem>
#include <HistoryThread>
#include <QDebug>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>

SQLiteHistoryWriter::SQLiteHistoryWriter(QObject *parent) :
    HistoryWriter(parent)
{
    SQLiteDatabase::instance();
}

HistoryThreadPtr SQLiteHistoryWriter::threadForParticipants(const QString &accountId, HistoryItem::ItemType type, const QStringList &participants)
{
    if (participants.isEmpty()) {
        return HistoryThreadPtr(0);
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
        return HistoryThreadPtr(0);
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
            return HistoryThreadPtr(0);
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

    if (existingThread.isNull()) {
        // Create a new thread
        // FIXME: define what the threadId will be
        QString threadId = participants.join("%");

        query.prepare("INSERT INTO threads (accountId, threadId, type, count, unreadCount)"
                      "VALUES (:accountId, :threadId, :type, :count, :unreadCount)");
        query.bindValue(":accountId", accountId);
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        query.bindValue(":count", 0);
        query.bindValue(":unreadCount", 0);
        if (!query.exec()) {
            qCritical() << "Error:" << query.lastError() << query.lastQuery();
            return HistoryThreadPtr(0);
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
                return HistoryThreadPtr(0);
            }
        }
        existingThread = threadId;
    }

    // and finally create the thread item
    // FIXME: check for existing instances of the thread object instead of always creating a new one
    HistoryThreadPtr thread(new HistoryThread(accountId, existingThread, participants));
    return thread;
}

bool SQLiteHistoryWriter::writeTextItem(const TextItem &item)
{
    qDebug() << "Going to write text item:" << item.accountId() << item.itemId() << item.sender() << item.message();

    QSqlQuery query(SQLiteDatabase::instance()->database());

    // FIXME: add support for checking if an item already exists

    query.prepare("INSERT INTO text_items (accountId, threadId, itemId, senderId, timestamp, newItem, message, messageType, messageFlags, readTimestamp) "
                  "VALUES (:accountId, :threadId, :itemId, :senderId, :timestamp, :newItem, :message, :messageType, :messageFlags, :readTimestamp)");
    query.bindValue(":accountId", item.accountId());
    query.bindValue(":threadId", item.threadId());
    query.bindValue(":itemId", item.itemId());
    query.bindValue(":senderId", item.sender());
    query.bindValue(":timestamp", item.timestamp());
    query.bindValue(":newItem", item.newItem());
    query.bindValue(":message", item.message());
    query.bindValue(":messageType", item.messageType());
    query.bindValue(":messageFlags", (int) item.messageFlags());
    query.bindValue(":readTimestamp", item.readTimestamp());

    if (!query.exec()) {
        qCritical() << "Failed to save the voice item. Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryWriter::writeVoiceItem(const VoiceItem &item)
{
    qDebug() << "Going to write voice item:" << item.accountId() << item.itemId() << item.sender();

    QSqlQuery query(SQLiteDatabase::instance()->database());

    // FIXME: add support for checking if an item already exists

    query.prepare("INSERT INTO voice_items (accountId, threadId, itemId, senderId, timestamp, newItem, duration, missed) "
                  "VALUES (:accountId, :threadId, :itemId, :senderId, :timestamp, :newItem, :duration, :missed)");
    query.bindValue(":accountId", item.accountId());
    query.bindValue(":threadId", item.threadId());
    query.bindValue(":itemId", item.itemId());
    query.bindValue(":senderId", item.sender());
    query.bindValue(":timestamp", item.timestamp());
    query.bindValue(":newItem", item.newItem());
    query.bindValue(":duration", QTime(0,0,0,0).secsTo(item.duration()));
    query.bindValue(":missed", item.missed());

    if (!query.exec()) {
        qCritical() << "Failed to save the voice item. Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}
