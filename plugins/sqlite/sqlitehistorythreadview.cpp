#include "sqlitehistorythreadview.h"
#include "sqlitedatabase.h"
#include "sqlitehistoryreader.h"
#include "thread.h"
#include "intersectionfilter.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDebug>
#include <QSqlError>

SQLiteHistoryThreadView::SQLiteHistoryThreadView(SQLiteHistoryReader *reader,
                                                 History::EventType type,
                                                 const History::SortPtr &sort,
                                                 const History::FilterPtr &filter)
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
           << "threads.lastEventId"
           << "threads.count"
           << "threads.unreadCount";

    QStringList extraFields;
    QString table;

    switch (type) {
    case History::EventTypeText:
        table = "text_events";
        extraFields << "text_events.message" << "text_events.messageType" << "text_events.messageFlags" << "text_events.readTimestamp";
        break;
    case History::EventTypeVoice:
        table = "voice_events";
        extraFields << "voice_events.duration" << "voice_events.missed";
        break;
    }

    fields << QString("%1.senderId").arg(table)
           << QString("%1.timestamp").arg(table)
           << QString("%1.newEvent").arg(table);
    fields << extraFields;

    QString queryText = QString("SELECT %1 FROM threads LEFT JOIN %2 ON threads.threadId=%2.threadId AND "
                                "threads.accountId=%2.accountId AND threads.lastEventId=%2.eventId WHERE threads.type=%3 %4")
                                .arg(fields.join(", "), table, QString::number((int)type), condition);

    // FIXME: add support for sorting
    if (!mQuery.exec(queryText)) {
        qCritical() << "Error:" << mQuery.lastError() << mQuery.lastQuery();
        return;
    }
}

QList<History::ThreadPtr> SQLiteHistoryThreadView::nextPage()
{
    QList<History::ThreadPtr> threads;
    int remaining = mPageSize;
    QSqlQuery secondaryQuery(SQLiteDatabase::instance()->database());

    while (mQuery.next() && remaining-- > 0) {
        QString accountId = mQuery.value(0).toString();
        QString threadId = mQuery.value(1).toString();
        QString lastEventId = mQuery.value(2).toString();
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
        History::EventPtr historyEvent;
        if (!lastEventId.isEmpty()) {
            switch (mType) {
            case History::EventTypeText:
                historyEvent = History::TextEventPtr(new History::TextEvent(accountId,
                                                                            threadId,
                                                                            lastEventId,
                                                                            mQuery.value(5).toString(),
                                                                            mQuery.value(6).toDateTime(),
                                                                            mQuery.value(7).toBool(),
                                                                            mQuery.value(8).toString(),
                                                                            (History::MessageType)mQuery.value(9).toInt(),
                                                                            History::MessageFlags(mQuery.value(10).toInt()),
                                                                            mQuery.value(11).toDateTime()));
                break;
            case History::EventTypeVoice:
                historyEvent = History::VoiceEventPtr(new History::VoiceEvent(accountId,
                                                                              threadId,
                                                                              lastEventId,
                                                                              mQuery.value(5).toString(),
                                                                              mQuery.value(6).toDateTime(),
                                                                              mQuery.value(7).toBool(),
                                                                              mQuery.value(9).toBool(),
                                                                              QTime(0,0).addSecs(mQuery.value(8).toInt())));
                break;
            }
        }
        // and last but not least, create the thread item and append it to the result set
        History::ThreadPtr thread(new History::Thread(accountId, threadId, mType, participants, historyEvent, count, unreadCount));
        threads << thread;
    }

    return threads;
}

bool SQLiteHistoryThreadView::isValid() const
{
    return mQuery.isActive();
}
