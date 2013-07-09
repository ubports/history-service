#include "sqlitehistoryreader.h"
#include "sqlitehistorythreadview.h"
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

HistoryThreadViewPtr SQLiteHistoryReader::queryThreads(HistoryItem::ItemType type,
                                                       const HistorySort &sort,
                                                       const HistoryFilter &filter)
{
    return HistoryThreadViewPtr(new SQLiteHistoryThreadView(this, type, sort, filter));
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
