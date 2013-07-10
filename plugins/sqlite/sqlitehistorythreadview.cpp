#include "sqlitehistorythreadview.h"
#include "sqlitedatabase.h"
#include "sqlitehistoryreader.h"
#include <QDebug>
#include <QSqlError>
#include <HistoryThread>
#include <HistoryIntersectionFilter>
#include <TextItem>
#include <VoiceItem>

SQLiteHistoryThreadView::SQLiteHistoryThreadView(SQLiteHistoryReader *reader,
                                                 HistoryItem::ItemType type,
                                                 const HistorySortPtr &sort,
                                                 const HistoryFilterPtr &filter)
    : mReader(reader), mType(type), mSort(sort), mFilter(filter), mPageSize(15),
      mQuery(SQLiteDatabase::instance()->database())
{
    // FIXME: sort the results property
    Q_UNUSED(sort)

    // FIXME: validate the filter
    QString condition;
    if (!filter.isNull()) {
        condition = filter->toString();
    }

    if (!condition.isEmpty()) {
        condition.prepend(" AND ");
    }

    QStringList fields;
    fields << "threads.accountId"
           << "threads.threadId"
           << "threads.lastItemId"
           << "threads.count"
           << "threads.unreadCount";

    QStringList extraFields;
    QString table;

    switch (type) {
    case HistoryItem::ItemTypeText:
        table = "text_items";
        extraFields << "text_items.message" << "text_items.messageType" << "text_items.messageFlags" << "text_items.readTimestamp";
        break;
    case HistoryItem::ItemTypeVoice:
        table = "voice_items";
        extraFields << "voice_items.duration" << "voice_items.missed";
        break;
    }

    fields << QString("%1.senderId").arg(table)
           << QString("%1.timestamp").arg(table)
           << QString("%1.newItem").arg(table);
    fields << extraFields;

    QString queryText = QString("SELECT %1 FROM threads LEFT JOIN %2 ON threads.threadId=%2.threadId AND "
                                "threads.accountId=%2.accountId AND threads.lastItemId=%2.itemId WHERE threads.type=%3 %4")
                                .arg(fields.join(", "), table, QString::number((int)type), condition);

    qDebug() << "Query text:" << queryText;

    // FIXME: add support for sorting
    if (!mQuery.exec(queryText)) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return;
    }
}

QList<HistoryThreadPtr> SQLiteHistoryThreadView::nextPage()
{
    QList<HistoryThreadPtr> threads;
    int remaining = mPageSize;
    QSqlQuery secondaryQuery(SQLiteDatabase::instance()->database());

    while (mQuery.next() && remaining-- > 0) {
        QString accountId = mQuery.value(0).toString();
        QString threadId = mQuery.value(1).toString();
        QString lastItemId = mQuery.value(2).toString();
        int count = mQuery.value(3).toInt();
        int unreadCount = mQuery.value(4).toInt();

        // now for each thread we need to fetch the participants
        secondaryQuery.prepare("SELECT participantId FROM thread_participants WHERE "
                               "accountId=:accountId AND threadId=:threadId AND type=:type");
        secondaryQuery.bindValue(":accountId", accountId);
        secondaryQuery.bindValue(":threadId", threadId);
        secondaryQuery.bindValue(":type", mType);
        if (!secondaryQuery.exec()) {
            qCritical() << "Error:" << secondaryQuery.lastError() << secondaryQuery.lastQuery();
            return threads;
        }

        QStringList participants;
        while (secondaryQuery.next()) {
            participants << secondaryQuery.value(0).toString();
        }

        // the next step is to get the last item
        HistoryItemPtr historyItem;
        if (!lastItemId.isEmpty()) {
            switch (mType) {
            case HistoryItem::ItemTypeText:
                historyItem = TextItemPtr(new TextItem(accountId,
                                                       threadId,
                                                       lastItemId,
                                                       mQuery.value(5).toString(),
                                                       mQuery.value(6).toDateTime(),
                                                       mQuery.value(7).toBool(),
                                                       mQuery.value(8).toString(),
                                                       (TextItem::MessageType)mQuery.value(9).toInt(),
                                                       TextItem::MessageFlags(mQuery.value(10).toInt()),
                                                       mQuery.value(11).toDateTime()));
                break;
            case HistoryItem::ItemTypeVoice:
                historyItem = VoiceItemPtr(new VoiceItem(accountId,
                                                         threadId,
                                                         lastItemId,
                                                         mQuery.value(5).toString(),
                                                         mQuery.value(6).toDateTime(),
                                                         mQuery.value(7).toBool(),
                                                         mQuery.value(9).toBool(),
                                                         QTime(0,0).addSecs(mQuery.value(8).toInt())));
                break;
            }
        }
        // and last but not least, create the thread item and append it to the result set
        HistoryThreadPtr thread(new HistoryThread(accountId, threadId, mType, participants, historyItem, count, unreadCount));
        threads << thread;
    }

    return threads;
}

bool SQLiteHistoryThreadView::isValid() const
{
    return mQuery.isActive();
}
