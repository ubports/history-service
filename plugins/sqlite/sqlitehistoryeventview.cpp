#include "sqlitehistoryeventview.h"
#include "sqlitedatabase.h"
#include "filter.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDebug>
#include <QSqlError>

SQLiteHistoryEventView::SQLiteHistoryEventView(SQLiteHistoryReader *reader,
                                             History::EventType type,
                                             const History::SortPtr &sort,
                                             const History::FilterPtr &filter)
    : mType(type), mSort(sort), mFilter(filter), mQuery(SQLiteDatabase::instance()->database()),
      mPageSize(15), mReader(reader)
{
    // FIXME: sort the results properly
    Q_UNUSED(sort)

    mQuery.setForwardOnly(true);

    // FIXME: validate the filter
    QString condition;
    if (!filter.isNull()) {
        condition = filter->toString();
    }
    if (!condition.isEmpty()) {
        condition.prepend(" WHERE ");
    }

    QString queryText;

    switch (type) {
    case History::EventTypeText:
        queryText = QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent,"
                            "message, messageType, messageFlags, readTimestamp FROM text_events %1").arg(condition);
        break;
    case History::EventTypeVoice:
        queryText = QString("SELECT accountId, threadId, eventId, senderId, timestamp, newEvent,"
                            "duration, missed FROM voice_events %1").arg(condition);
        break;
    }

    if (!mQuery.exec(queryText)) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return;
    }
}

QList<History::EventPtr> SQLiteHistoryEventView::nextPage()
{
    QList<History::EventPtr> events;
    int remaining = mPageSize;

    while (mQuery.next() && remaining-- > 0) {
        switch (mType) {
        case History::EventTypeText:
            events << History::EventPtr(new History::TextEvent(mQuery.value(0).toString(),
                                                               mQuery.value(1).toString(),
                                                               mQuery.value(2).toString(),
                                                               mQuery.value(3).toString(),
                                                               mQuery.value(4).toDateTime(),
                                                               mQuery.value(5).toBool(),
                                                               mQuery.value(6).toString(),
                                                               (History::MessageType) mQuery.value(7).toInt(),
                                                               (History::MessageFlags) mQuery.value(8).toInt(),
                                                               mQuery.value(9).toDateTime()));
            break;
        case History::EventTypeVoice:
            events << History::EventPtr(new History::VoiceEvent(mQuery.value(0).toString(),
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

    return events;
}

bool SQLiteHistoryEventView::isValid() const
{
    return mQuery.isActive();
}
