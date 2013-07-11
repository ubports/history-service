#include "historythreadmodel.h"
#include "thread.h"
#include "historyqmlfilter.h"
#include "manager.h"
#include "threadview.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDebug>

HistoryThreadModel::HistoryThreadModel(QObject *parent) :
    QAbstractListModel(parent), mCanFetchMore(true), mFilter(0), mType(EventTypeText)
{
    // configure the roles
    mRoles[AccountIdRole] = "accountId";
    mRoles[ThreadIdRole] = "threadId";
    mRoles[TypeRole] = "type";
    mRoles[ParticipantsRole] = "participants";
    mRoles[CountRole] = "count";
    mRoles[UnreadCountRole] = "unreadCount";

    // roles related to the thread´s last item
    mRoles[LastItemIdRole] = "eventId";
    mRoles[LastItemSenderRole] = "itemSender";
    mRoles[LastItemTimestampRole] = "itemTimestamp";
    mRoles[LastItemNewRole] = "itemNew";
    mRoles[LastItemTextMessageRole] = "itemTextMessage";
    mRoles[LastItemTextMessageTypeRole] = "itemTextMessageType";
    mRoles[LastItemTextMessageFlagsRole] = "itemTextMessageFlags";
    mRoles[LastItemTextReadTimestampRole] = "itemTextReadTimestamp";
    mRoles[LastItemCallMissedRole] = "itemCallMissed";
    mRoles[LastItemCallDurationRole] = "itemCallDuration";

    // create the results view
    updateQuery();
}

int HistoryThreadModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mThreads.count();
}

QVariant HistoryThreadModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mThreads.count()) {
        return QVariant();
    }

    History::ThreadPtr thread = mThreads[index.row()];
    History::EventPtr event = thread->lastEvent();
    History::TextEventPtr textEvent;
    History::VoiceEventPtr voiceEvent;

    if (!event.isNull()) {
        switch (event->type()) {
        case History::EventTypeText:
            textEvent = event.staticCast<History::TextEvent>();
            break;
        case History::EventTypeVoice:
            voiceEvent = event.staticCast<History::VoiceEvent>();
            break;
        }
    }

    QVariant result;
    switch (role) {
    case AccountIdRole:
        result = thread->accountId();
        break;
    case ThreadIdRole:
        result = thread->threadId();
        break;
    case TypeRole:
        result = (int) thread->type();
        break;
    case ParticipantsRole:
        result = thread->participants();
        break;
    case CountRole:
        result = thread->count();
        break;
    case UnreadCountRole:
        result = thread->unreadCount();
        break;
    case LastItemIdRole:
        if (!event.isNull()) {
            result = event->eventId();
        }
        break;
    case LastItemSenderRole:
        if (!event.isNull()) {
            result = event->sender();
        }
        break;
    case LastItemTimestampRole:
        if (!event.isNull()) {
            result = event->timestamp();
        }
        break;
    case LastItemNewRole:
        if (!event.isNull()) {
            result = event->newEvent();
        }
        break;
    case LastItemTextMessageRole:
        if (!textEvent.isNull()) {
            result = textEvent->message();
        }
        break;
    case LastItemTextMessageTypeRole:
        if (!textEvent.isNull()) {
            result = (int) textEvent->messageType();
        }
        break;
    case LastItemTextMessageFlagsRole:
        if (!textEvent.isNull()) {
            result = (int) textEvent->messageFlags();
        }
        break;
    case LastItemTextReadTimestampRole:
        if (!textEvent.isNull()) {
            result = textEvent->readTimestamp();
        }
        break;
    case LastItemCallMissedRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent->missed();
        }
        break;
    case LastItemCallDurationRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent->duration();
        }
        break;
    }

    return result;
}

bool HistoryThreadModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }

    return mCanFetchMore;
}

void HistoryThreadModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }

    QList<History::ThreadPtr> threads = mThreadView->nextPage();
    qDebug() << "Got items:" << threads.count();
    // if the number of returned items is less than the page size, it means we have reached the end
    // and cannot fetch more items
    if (threads.isEmpty()) {
        mCanFetchMore = false;
    }

    beginInsertRows(QModelIndex(), mThreads.count(), mThreads.count() + threads.count() - 1);
    mThreads << threads;
    endInsertRows();
}

QHash<int, QByteArray> HistoryThreadModel::roleNames() const
{
    return mRoles;
}

HistoryQmlFilter *HistoryThreadModel::filter() const
{
    return mFilter;
}

void HistoryThreadModel::setFilter(HistoryQmlFilter *value)
{
    // disconnect the previous filter
    if (mFilter) {
        mFilter->disconnect(this);
    }

    mFilter = value;
    if (mFilter) {
        connect(mFilter,
                SIGNAL(filterChanged()),
                SLOT(updateQuery()));
    }

    Q_EMIT filterChanged();
    updateQuery();
}

HistoryThreadModel::EventType HistoryThreadModel::type() const
{
    return mType;
}

void HistoryThreadModel::setType(HistoryThreadModel::EventType value)
{
    mType = value;
    Q_EMIT typeChanged();
    updateQuery();
}


void HistoryThreadModel::updateQuery()
{
    // remove all items from the model
    beginRemoveRows(QModelIndex(), 0, mThreads.count() - 1);
    mThreads.clear();
    endRemoveRows();

    // and fetch again
    mCanFetchMore = true;

    History::FilterPtr queryFilter;
    History::SortPtr querySort;

    if (mFilter) {
        queryFilter = mFilter->filter();
    }
    mThreadView = History::Manager::instance()->queryThreads((History::EventType)mType, querySort, queryFilter);
    fetchMore(QModelIndex());
}
