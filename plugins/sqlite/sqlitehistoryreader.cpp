#include "sqlitehistoryreader.h"
#include "sqlitedatabase.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <HistoryFilter>
#include <HistoryIntersectionFilter>
#include <HistoryThread>
#include <TextItem>
#include <VoiceItem>

SQLiteHistoryReader::SQLiteHistoryReader(QObject *parent) :
    HistoryReader(parent)
{
}

QList<HistoryThreadPtr> SQLiteHistoryReader::queryThreads(HistoryItem::ItemType type,
                                                          const HistorySort &sort,
                                                          const HistoryFilter &filter,
                                                          int startOffset,
                                                          int pageSize)
{
    QList<HistoryThreadPtr> threads;
    QSqlQuery query(SQLiteDatabase::instance()->database());

    // FIXME: sort the results property
    Q_UNUSED(sort)

    // FIXME: validate the filter
    QString condition = filter.toString();
    if (!condition.isEmpty()) {
        condition.prepend(" AND ");
    }

    QString queryText = QString("SELECT accountId, threadId, lastItemId, count, unreadCount FROM threads "
                                "WHERE type=%1 %2 %3")
                                .arg(QString::number((int)type), condition, pageSqlCommand(startOffset, pageSize));

    // FIXME: add support for sorting
    if (!query.exec(queryText)) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return threads;
    }

    QSqlQuery secondaryQuery(SQLiteDatabase::instance()->database());
    while (query.next()) {
        QString accountId = query.value(0).toString();
        QString threadId = query.value(1).toString();
        QString lastItemId = query.value(2).toString();
        int count = query.value(3).toInt();
        int unreadCount = query.value(4).toInt();

        // now for each thread we need to fetch the participants
        secondaryQuery.prepare("SELECT participantId FROM thread_participants WHERE "
                               "accountId=:accountId AND threadId=:threadId AND type=:type");
        secondaryQuery.bindValue(":accountId", accountId);
        secondaryQuery.bindValue(":threadId", threadId);
        secondaryQuery.bindValue(":type", type);
        if (!secondaryQuery.exec()) {
            qCritical() << "Error:" << secondaryQuery.lastError() << query.lastQuery();
            return threads;
        }

        QStringList participants;
        while (secondaryQuery.next()) {
            participants << secondaryQuery.value(0).toString();
        }

        // the next step is to get the last item
        HistoryItemPtr historyItem;
        HistoryIntersectionFilter filter;
        filter.append(HistoryFilter("accountId", accountId));
        filter.append(HistoryFilter("threadId", threadId));
        filter.append(HistoryFilter("itemId", lastItemId));

        QList<HistoryItemPtr> items = queryItems(type, HistorySort(), filter);
        if (!items.isEmpty()) {
            historyItem = items.first();
        }

        // and last but not least, create the thread item and append it to the result set
        HistoryThreadPtr thread(new HistoryThread(accountId, threadId, type, participants, historyItem, count, unreadCount));
        threads << thread;
    }

    return threads;
}

QList<HistoryItemPtr> SQLiteHistoryReader::queryItems(HistoryItem::ItemType type,
                                                      const HistorySort &sort,
                                                      const HistoryFilter &filter,
                                                      int startOffset,
                                                      int pageSize)
{
    switch (type) {
    case HistoryItem::ItemTypeText:
        return queryTextItems(sort, filter, startOffset, pageSize);
    case HistoryItem::ItemTypeVoice:
        return queryVoiceItems(sort, filter, startOffset, pageSize);
    }

    return QList<HistoryItemPtr>();
}

QList<HistoryItemPtr> SQLiteHistoryReader::queryTextItems(const HistorySort &sort,
                                                          const HistoryFilter &filter,
                                                          int startOffset,
                                                          int pageSize)
{
    QList<HistoryItemPtr> items;

    // FIXME: sort the results properly
    Q_UNUSED(sort)

    // FIXME: validate the filter
    QString condition = filter.toString();
    if (!condition.isEmpty()) {
        condition.prepend(" WHERE ");
    }

    QString queryText = QString("SELECT accountId, threadId, itemId, senderId, timestamp, newItem,"
                                "message, messageType, messageFlags, readTimestamp FROM text_items %1 %2")
                                .arg(condition, pageSqlCommand(startOffset, pageSize));
    QSqlQuery query(SQLiteDatabase::instance()->database());
    if (!query.exec(queryText)) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return items;
    }

    while (query.next()) {
        TextItemPtr textItem(new TextItem(query.value(0).toString(),
                                          query.value(1).toString(),
                                          query.value(2).toString(),
                                          query.value(3).toString(),
                                          query.value(4).toDateTime(),
                                          query.value(5).toBool(),
                                          query.value(6).toString(),
                                          (TextItem::MessageType) query.value(7).toInt(),
                                          (TextItem::MessageFlags) query.value(8).toInt(),
                                          query.value(9).toDateTime()));
        items << textItem;
    }
    return items;
}

QList<HistoryItemPtr> SQLiteHistoryReader::queryVoiceItems(const HistorySort &sort,
                                                           const HistoryFilter &filter,
                                                           int startOffset,
                                                           int pageSize)
{
    QList<HistoryItemPtr> items;

    // FIXME: sort the results properly
    Q_UNUSED(sort)

    // FIXME: validate the filter
    QString condition = filter.toString();
    if (!condition.isEmpty()) {
        condition.prepend(" WHERE ");
    }

    QString queryText = QString("SELECT accountId, threadId, itemId, senderId, timestamp, newItem,"
                                "duration, missed FROM voice_items %1 %2").arg(condition, pageSqlCommand(startOffset, pageSize));
    QSqlQuery query(SQLiteDatabase::instance()->database());
    if (!query.exec(queryText)) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return items;
    }

    while (query.next()) {
        VoiceItemPtr voiceItem(new VoiceItem(query.value(0).toString(),
                                             query.value(1).toString(),
                                             query.value(2).toString(),
                                             query.value(3).toString(),
                                             query.value(4).toDateTime(),
                                             query.value(5).toBool(),
                                             query.value(7).toBool(),
                                             QTime(0,0).addSecs(query.value(6).toInt())));
        items << voiceItem;
    }
    return items;
}

QString SQLiteHistoryReader::pageSqlCommand(int startOffset, int pageSize) const
{
    QString paging;
    if (pageSize > 0) {
        paging = QString("LIMIT %1").arg(QString::number(pageSize));
    }

    if (startOffset > 0) {
        paging += QString(" OFFSET %1").arg(QString::number(startOffset));
    }

    return paging;
}
