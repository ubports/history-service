#include "historythreadmodel.h"
#include "historythread.h"
#include "historyqmlfilter.h"
#include <HistoryManager>
#include <QDebug>

HistoryThreadModel::HistoryThreadModel(QObject *parent) :
    QAbstractListModel(parent), mCanFetchMore(true), mPageSize(15), mFilter(0), mType(VoiceItem)
{
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
    QHash<int, QByteArray> roles;
    roles[AccountIdRole] = "accountId";
    roles[ThreadIdRole] = "threadId";
    roles[TypeRole] = "type";
    roles[ParticipantsRole] = "participants";
    roles[CountRole] = "count";
    roles[UnreadCountRole] = "unreadCount";

    return roles;
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
