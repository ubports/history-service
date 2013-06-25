#include "sqlitehistorywriter.h"
#include "sqlitedatabase.h"
#include <HistoryItem>
#include <HistoryThread>
#include <QDebug>
#include <QSqlQuery>
#include <QVariant>

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

    // select all the threads the first participant is listed in, and from that list
    // check if any of the threads has all the other participants listed
    // FIXME: find a better way to do this
    QStringList conditions;

    conditions << QString("participantId=\"%1\"").arg(participants[0])
               << QString("type=%1").arg(type)
               << QString("accountId=%1").arg(accountId);

    QSqlQuery query(QString("SELECT threadId FROM thread_participants WHERE %1").arg(conditions.join(" AND ")), SQLiteDatabase::instance()->database());
    if (!query.exec()) {
        return HistoryThreadPtr(0);
    }

    QStringList threadIds;
    while (query.next()) {
        threadIds << query.value(0).toString();
    }

    QString existingThread;
    // now for each threadId, check if all the other participants are listed
    Q_FOREACH(const QString &threadId, threadIds) {
        conditions.clear();
        conditions << QString("threadId=\"%1\"").arg(threadId)
                   << QString("type=%1").arg(type)
                   << QString("accountId=%1").arg(accountId);
        query = QSqlQuery(QString("SELECT participantId FROM thread_participants WHERE %1").arg(conditions.join(" AND ")),
                          SQLiteDatabase::instance()->database());
        if (!query.exec()) {
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

        query = QSqlQuery("INSERT INTO threads (accountId, threadId, type, count, unreadCount)"
                          "VALUES (:accountId, :threadId, :type, :count, :unreadCount)",
                          SQLiteDatabase::instance()->database());
        query.bindValue(":accountId", accountId);
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        query.bindValue(":count", 1);
        query.bindValue(":unreadCount", 0);
        if (!query.exec()) {
            return HistoryThreadPtr(0);
        }

        // and insert the participants
        Q_FOREACH(const QString &participant, participants) {
            query = QSqlQuery("INSERT INTO thread_participants (accountId, threadId, type, participantId)"
                              "VALUES (:accountId, :threadId, :type, :participantId)",
                              SQLiteDatabase::instance()->database());
            query.bindValue(":accountId", accountId);
            query.bindValue(":threadId", threadId);
            query.bindValue(":type", type);
            query.bindValue(":participantId", participant);
            if (!query.exec()) {
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
}

bool SQLiteHistoryWriter::writeVoiceItem(const VoiceItem &item)
{
    qDebug() << "Going to write voice item:" << item.accountId() << item.itemId() << item.sender();
}
