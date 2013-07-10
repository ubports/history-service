#include "sqlitehistorythreadview.h"
#include "sqlitedatabase.h"
#include "sqlitehistoryreader.h"
#include <QDebug>
#include <QSqlError>
#include <HistoryThread>
#include <HistoryIntersectionFilter>

SQLiteHistoryThreadView::SQLiteHistoryThreadView(SQLiteHistoryReader *reader,
                                                 HistoryItem::ItemType type,
                                                 const HistorySort &sort,
                                                 const HistoryFilter &filter)
    : mReader(reader), mType(type), mSort(sort), mFilter(filter), mPageSize(15),
      mQuery(SQLiteDatabase::instance()->database())
{
    // FIXME: sort the results property
    Q_UNUSED(sort)

    // FIXME: validate the filter
    QString condition = filter.toString();
    if (!condition.isEmpty()) {
        condition.prepend(" AND ");
    }

    QString queryText = QString("SELECT accountId, threadId, lastItemId, count, unreadCount FROM threads "
                                "WHERE type=%1 %2")
                                .arg(QString::number((int)type), condition);

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
        HistoryIntersectionFilter filter;
        filter.append(HistoryFilter("accountId", accountId));
        filter.append(HistoryFilter("threadId", threadId));
        filter.append(HistoryFilter("itemId", lastItemId));

        /* FIXME: port to the new API
         *QList<HistoryItemPtr> items = mReader->queryItems(mType, HistorySort(), filter);
        if (!items.isEmpty()) {
            historyItem = items.first();
        }*/

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
