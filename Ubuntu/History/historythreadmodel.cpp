#include "historythreadmodel.h"
#include "historythread.h"
#include "historyqmlfilter.h"
#include <HistoryManager>
#include <TextItem>
#include <VoiceItem>
#include <QDebug>

HistoryThreadModel::HistoryThreadModel(QObject *parent) :
    QAbstractListModel(parent), mCanFetchMore(true), mPageSize(15), mFilter(0), mType(ItemTypeText)
{
    // configure the roles
    mRoles[AccountIdRole] = "accountId";
    mRoles[ThreadIdRole] = "threadId";
    mRoles[TypeRole] = "type";
    mRoles[ParticipantsRole] = "participants";
    mRoles[CountRole] = "count";
    mRoles[UnreadCountRole] = "unreadCount";

    // roles related to the thread´s last item
    mRoles[LastItemIdRole] = "itemId";
    mRoles[LastItemSenderRole] = "itemSender";
    mRoles[LastItemTimestampRole] = "itemTimestamp";
    mRoles[LastItemNewRole] = "itemNew";
    mRoles[LastItemTextMessageRole] = "itemTextMessage";
    mRoles[LastItemTextMessageTypeRole] = "itemTextMessageType";
    mRoles[LastItemTextMessageFlagsRole] = "itemTextMessageFlags";
    mRoles[LastItemTextReadTimestampRole] = "itemTextReadTimestamp";
    mRoles[LastItemCallMissedRole] = "itemCallMissed";
    mRoles[LastItemCallDurationRole] = "itemCallDuration";

    // fetch an initial page of results
    fetchMore(QModelIndex());
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
    if (!index.isValid() || index.row() < 0 || index.row() > mThreads.count()) {
        return QVariant();
    }

    HistoryThreadPtr thread = mThreads[index.row()];
    HistoryItemPtr item = thread->lastItem();
    TextItemPtr textItem;
    VoiceItemPtr voiceItem;

    if (!item.isNull()) {
        switch (item->type()) {
        case HistoryItem::ItemTypeText:
            textItem = item.staticCast<TextItem>();
            break;
        case HistoryItem::ItemTypeVoice:
            voiceItem = item.staticCast<VoiceItem>();
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
        if (!item.isNull()) {
            result = item->itemId();
        }
        break;
    case LastItemSenderRole:
        if (!item.isNull()) {
            result = item->sender();
        }
        break;
    case LastItemTimestampRole:
        if (!item.isNull()) {
            result = item->timestamp();
        }
        break;
    case LastItemNewRole:
        if (!item.isNull()) {
            result = item->newItem();
        }
        break;
    case LastItemTextMessageRole:
        if (!textItem.isNull()) {
            result = textItem->message();
        }
        break;
    case LastItemTextMessageTypeRole:
        if (!textItem.isNull()) {
            result = (int) textItem->messageType();
        }
        break;
    case LastItemTextMessageFlagsRole:
        if (!textItem.isNull()) {
            result = (int) textItem->messageFlags();
        }
        break;
    case LastItemTextReadTimestampRole:
        if (!textItem.isNull()) {
            result = textItem->readTimestamp();
        }
        break;
    case LastItemCallMissedRole:
        if (!voiceItem.isNull()) {
            result = voiceItem->missed();
        }
        break;
    case LastItemCallDurationRole:
        if (!voiceItem.isNull()) {
            result = voiceItem->duration();
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

    HistoryFilter queryFilter;
    HistorySort querySort;

    if (mFilter) {
        queryFilter = mFilter->filter();
    }

    QList<HistoryThreadPtr> threads = HistoryManager::instance()->queryThreads((HistoryItem::ItemType)mType,
                                                                               querySort,
                                                                               queryFilter,
                                                                               mThreads.count(),
                                                                               mPageSize);

    qDebug() << "Got items:" << threads.count();
    // if the number of returned items is less than the page size, it means we have reached the end
    // and cannot fetch more items
    if (threads.count() < mPageSize) {
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

HistoryThreadModel::ItemType HistoryThreadModel::type() const
{
    return mType;
}

void HistoryThreadModel::setType(HistoryThreadModel::ItemType value)
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
    fetchMore(QModelIndex());
}
