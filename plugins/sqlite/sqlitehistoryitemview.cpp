#include "sqlitehistoryitemview.h"
#include "sqlitedatabase.h"
#include <TextItem>
#include <VoiceItem>
#include <QDebug>
#include <QSqlError>

SQLiteHistoryItemView::SQLiteHistoryItemView(SQLiteHistoryReader *reader,
                                             HistoryItem::ItemType type,
                                             const HistorySort &sort,
                                             const HistoryFilter &filter)
    : mType(type), mSort(sort), mFilter(filter), mQuery(SQLiteDatabase::instance()->database()),
      mPageSize(15), mReader(reader)
{
    // FIXME: sort the results properly
    Q_UNUSED(sort)

    // FIXME: validate the filter
    QString condition = filter.toString();
    if (!condition.isEmpty()) {
        condition.prepend(" WHERE ");
    }

    QString queryText;

    switch (type) {
    case HistoryItem::ItemTypeText:
        queryText = QString("SELECT accountId, threadId, itemId, senderId, timestamp, newItem,"
                            "message, messageType, messageFlags, readTimestamp FROM text_items %1").arg(condition);
        break;
    case HistoryItem::ItemTypeVoice:
        queryText = QString("SELECT accountId, threadId, itemId, senderId, timestamp, newItem,"
                            "duration, missed FROM voice_items %1").arg(condition);
        break;
    }

    if (!mQuery.exec(queryText)) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return;
    }
}

QList<HistoryItemPtr> SQLiteHistoryItemView::nextPage()
{
    QList<HistoryItemPtr> items;
    int remaining = mPageSize;

    while (mQuery.next() && remaining-- > 0) {
        switch (mType) {
        case HistoryItem::ItemTypeText:
            items << HistoryItemPtr(new TextItem(mQuery.value(0).toString(),
                                              mQuery.value(1).toString(),
                                              mQuery.value(2).toString(),
                                              mQuery.value(3).toString(),
                                              mQuery.value(4).toDateTime(),
                                              mQuery.value(5).toBool(),
                                              mQuery.value(6).toString(),
                                              (TextItem::MessageType) mQuery.value(7).toInt(),
                                              (TextItem::MessageFlags) mQuery.value(8).toInt(),
                                              mQuery.value(9).toDateTime()));
            break;
        case HistoryItem::ItemTypeVoice:
            items << HistoryItemPtr(new VoiceItem(mQuery.value(0).toString(),
                                                mQuery.value(1).toString(),
                                                mQuery.value(2).toString(),
                                                mQuery.value(3).toString(),
                                                mQuery.value(4).toDateTime(),
                                                mQuery.value(5).toBool(),
                                                mQuery.value(7).toBool(),
                                                QTime(0,0).addSecs(mQuery.value(6).toInt())));
            break;
        }
    }

    return items;
}

bool SQLiteHistoryItemView::isValid() const
{
    return mQuery.isActive();
}
