#include "historyeventmodel.h"
#include "historyqmlfilter.h"
#include "eventview.h"
#include "manager.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDebug>

HistoryEventModel::HistoryEventModel(QObject *parent) :
    QAbstractListModel(parent), mCanFetchMore(true), mFilter(0),
    mType(HistoryThreadModel::EventTypeText)
{
    // configure the roles
    mRoles[AccountIdRole] = "accountId";
    mRoles[ThreadIdRole] = "threadId";
    mRoles[TypeRole] = "type";
    mRoles[ItemIdRole] = "eventId";
    mRoles[SenderRole] = "sender";
    mRoles[TimestampRole] = "timestamp";
    mRoles[NewItemRole] = "newEvent";
    mRoles[TextMessageRole] = "textMessage";
    mRoles[TextMessageTypeRole] = "textMessageType";
    mRoles[TextMessageFlagsRole] = "textMessageFlags";
    mRoles[TextReadTimestampRole] = "textReadTimestamp";
    mRoles[CallMissedRole] = "callMissed";
    mRoles[CallDurationRole] = "callDuration";

    // create the view and get some objects
    updateQuery();
}

int HistoryEventModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mEvents.count();
}

QVariant HistoryEventModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= mEvents.count()) {
        return QVariant();
    }

    History::EventPtr event = mEvents[index.row()];
    History::TextEventPtr textEvent;
    History::VoiceEventPtr voiceEvent;

    switch (event->type()) {
    case History::EventTypeText:
        textEvent = event.staticCast<History::TextEvent>();
        break;
    case History::EventTypeVoice:
        voiceEvent = event.staticCast<History::VoiceEvent>();
        break;
    }

    QVariant result;

    switch (role) {
    case AccountIdRole:
        result = event->accountId();
        break;
    case ThreadIdRole:
        result = event->threadId();
        break;
    case TypeRole:
        result = event->type();
        break;
    case ItemIdRole:
        result = event->eventId();
        break;
    case SenderRole:
        result = event->sender();
        break;
    case TimestampRole:
        result = event->timestamp();
        break;
    case NewItemRole:
        result = event->newEvent();
        break;
    case TextMessageRole:
        if (!textEvent.isNull()) {
            result = textEvent->message();
        }
        break;
    case TextMessageTypeRole:
        if (!textEvent.isNull()) {
            result = (int)textEvent->messageType();
        }
        break;
    case TextMessageFlagsRole:
        if (!textEvent.isNull()) {
            result = (int)textEvent->messageFlags();
        }
        break;
    case TextReadTimestampRole:
        if (!textEvent.isNull()) {
            result = textEvent->readTimestamp();
        }
        break;
    case CallMissedRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent->missed();
        }
        break;
    case CallDurationRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent->duration();
        }
        break;
    }

    return result;
}

bool HistoryEventModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return false;
    }

    return mCanFetchMore;
}

void HistoryEventModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid()) {
        return;
    }

    QList<History::EventPtr> events = mView->nextPage();

    qDebug() << "Got events:" << events.count();
    // if the number of returned items is less than the page size, it means we have reached the end
    // and cannot fetch more items
    if (events.isEmpty()) {
        mCanFetchMore = false;
    }

    beginInsertRows(QModelIndex(), mEvents.count(), mEvents.count() + events.count() - 1);
    mEvents << events;
    endInsertRows();
}

QHash<int, QByteArray> HistoryEventModel::roleNames() const
{
    return mRoles;
}

HistoryQmlFilter *HistoryEventModel::filter() const
{
    return mFilter;
}

void HistoryEventModel::setFilter(HistoryQmlFilter *value)
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

HistoryThreadModel::EventType HistoryEventModel::type() const
{
    return mType;
}

void HistoryEventModel::setType(HistoryThreadModel::EventType value)
{
    mType = value;
    Q_EMIT typeChanged();
    updateQuery();
}


void HistoryEventModel::updateQuery()
{
    // remove all items from the model
    beginRemoveRows(QModelIndex(), 0, mEvents.count() - 1);
    mEvents.clear();
    endRemoveRows();

    // and create the view again
    History::FilterPtr queryFilter;
    History::SortPtr querySort;

    if (mFilter) {
        queryFilter = mFilter->filter();
    }

    mView = History::Manager::instance()->queryEvents((History::EventType)mType, querySort, queryFilter);
    mCanFetchMore = true;

    // get an initial set of results
    fetchMore(QModelIndex());
}
