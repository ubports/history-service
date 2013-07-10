#include "historyitemmodel.h"
#include "historyqmlfilter.h"
#include <HistoryItemView>
#include <HistorySort>
#include <HistoryManager>
#include <Types>
#include <TextItem>
#include <VoiceItem>
#include <QDebug>

HistoryItemModel::HistoryItemModel(QObject *parent) :
    QAbstractListModel(parent), mCanFetchMore(true), mFilter(0),
    mType(HistoryThreadModel::ItemTypeText)
{
    // configure the roles
    mRoles[AccountIdRole] = "accountId";
    mRoles[ThreadIdRole] = "threadId";
    mRoles[TypeRole] = "type";
    mRoles[ItemIdRole] = "itemId";
    mRoles[SenderRole] = "sender";
    mRoles[TimestampRole] = "timestamp";
    mRoles[NewItemRole] = "newItem";
    mRoles[TextMessageRole] = "textMessage";
    mRoles[TextMessageTypeRole] = "textMessageType";
    mRoles[TextMessageFlagsRole] = "textMessageFlags";
    mRoles[TextReadTimestampRole] = "textReadTimestamp";
    mRoles[CallMissedRole] = "callMissed";
    mRoles[CallDurationRole] = "callDuration";

    // create the view and get some objects
    updateQuery();
}

int HistoryItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mItems.count();
}

QVariant HistoryItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mItems.count()) {
        return QVariant();
    }

    HistoryItemPtr item = mItems[index.row()];
    TextItemPtr textItem;
    VoiceItemPtr voiceItem;

    switch (item->type()) {
    case HistoryItem::ItemTypeText:
        textItem = item.staticCast<TextItem>();
        break;
    case HistoryItem::ItemTypeVoice:
        voiceItem = item.staticCast<VoiceItem>();
        break;
    }

    QVariant result;

    switch (role) {
    case AccountIdRole:
        result = item->accountId();
        break;
    case ThreadIdRole:
        result = item->threadId();
        break;
    case TypeRole:
        result = item->type();
        break;
    case ItemIdRole:
        result = item->itemId();
        break;
    case SenderRole:
        result = item->sender();
        break;
    case TimestampRole:
        result = item->timestamp();
        break;
    case NewItemRole:
        result = item->newItem();
        break;
    case TextMessageRole:
        if (!textItem.isNull()) {
            result = textItem->message();
        }
        break;
    case TextMessageTypeRole:
        if (!textItem.isNull()) {
            result = (int)textItem->messageType();
        }
        break;
    case TextMessageFlagsRole:
        if (!textItem.isNull()) {
            result = (int)textItem->messageFlags();
        }
        break;
    case TextReadTimestampRole:
        if (!textItem.isNull()) {
            result = textItem->readTimestamp();
        }
        break;
    case CallMissedRole:
        if (!voiceItem.isNull()) {
            result = voiceItem->missed();
        }
        break;
    case CallDurationRole:
        if (!voiceItem.isNull()) {
            result = voiceItem->duration();
        }
        break;
    }

    return result;
}

bool HistoryItemModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }

    return mCanFetchMore;
}

void HistoryItemModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }

    QList<HistoryItemPtr> items = mView->nextPage();

    qDebug() << "Got items:" << items.count();
    // if the number of returned items is less than the page size, it means we have reached the end
    // and cannot fetch more items
    if (items.isEmpty()) {
        mCanFetchMore = false;
    }

    beginInsertRows(QModelIndex(), mItems.count(), mItems.count() + items.count() - 1);
    mItems << items;
    endInsertRows();
}

QHash<int, QByteArray> HistoryItemModel::roleNames() const
{
    return mRoles;
}

HistoryQmlFilter *HistoryItemModel::filter() const
{
    return mFilter;
}

void HistoryItemModel::setFilter(HistoryQmlFilter *value)
{
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

HistoryThreadModel::ItemType HistoryItemModel::type() const
{
    return mType;
}

void HistoryItemModel::setType(HistoryThreadModel::ItemType value)
{
    mType = value;
    Q_EMIT typeChanged();
    updateQuery();
}


void HistoryItemModel::updateQuery()
{
    // remove all items from the model
    beginRemoveRows(QModelIndex(), 0, mItems.count() - 1);
    mItems.clear();
    endRemoveRows();

    // and create the view again
    HistoryFilterPtr queryFilter;
    HistorySortPtr querySort;

    if (mFilter) {
        queryFilter = mFilter->filter();
    }

    mView = HistoryManager::instance()->queryItems((HistoryItem::ItemType)mType, querySort, queryFilter);
    mCanFetchMore = true;

    // get an initial set of results
    fetchMore(QModelIndex());
}
