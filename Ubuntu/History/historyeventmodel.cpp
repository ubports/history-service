/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    mRoles[EventIdRole] = "eventId";
    mRoles[SenderIdRole] = "senderId";
    mRoles[TimestampRole] = "timestamp";
    mRoles[NewEventRole] = "newEvent";
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
    case EventIdRole:
        result = event->eventId();
        break;
    case SenderIdRole:
        result = event->senderId();
        break;
    case TimestampRole:
        result = event->timestamp();
        break;
    case NewEventRole:
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

    History::Events events = mView->nextPage();

    qDebug() << "Got events:" << events.count();
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
    // remove all events from the model
    beginRemoveRows(QModelIndex(), 0, mEvents.count() - 1);
    mEvents.clear();
    endRemoveRows();

    // and create the view again
    History::FilterPtr queryFilter;
    History::SortPtr querySort;

    if (!mView.isNull()) {
        mView->disconnect(this);
    }

    if (mFilter) {
        queryFilter = mFilter->filter();
    }

    mView = History::Manager::instance()->queryEvents((History::EventType)mType, querySort, queryFilter);
    connect(mView.data(),
            SIGNAL(eventsAdded(History::Events)),
            SLOT(onEventsAdded(History::Events)));
    connect(mView.data(),
            SIGNAL(eventsModified(History::Events)),
            SLOT(onEventsModified(History::Events)));
    connect(mView.data(),
            SIGNAL(eventsRemoved(History::Events)),
            SLOT(onEventsRemoved(History::Events)));
    mCanFetchMore = true;

    // get an initial set of results
    fetchMore(QModelIndex());
}

void HistoryEventModel::onEventsAdded(const History::Events &events)
{
    if (!events.count()) {
        return;
    }

    //FIXME: handle sorting
    beginInsertRows(QModelIndex(), mEvents.count(), mEvents.count() + events.count() - 1);
    mEvents << events;
    endInsertRows();
}

void HistoryEventModel::onEventsModified(const History::Events &events)
{
    // FIXME: implement
}

void HistoryEventModel::onEventsRemoved(const History::Events &events)
{
    // FIXME: implement
}
