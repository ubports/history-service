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

    // roles related to the thread´s last event
    mRoles[LastEventIdRole] = "eventId";
    mRoles[LastEventSenderIdRole] = "eventSenderId";
    mRoles[LastEventTimestampRole] = "eventTimestamp";
    mRoles[LastEventNewRole] = "eventNew";
    mRoles[LastEventTextMessageRole] = "eventTextMessage";
    mRoles[LastEventTextMessageTypeRole] = "eventTextMessageType";
    mRoles[LastEventTextMessageFlagsRole] = "eventTextMessageFlags";
    mRoles[LastEventTextReadTimestampRole] = "eventTextReadTimestamp";
    mRoles[LastEventCallMissedRole] = "eventCallMissed";
    mRoles[LastEventCallDurationRole] = "eventCallDuration";

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
    case LastEventIdRole:
        if (!event.isNull()) {
            result = event->eventId();
        }
        break;
    case LastEventSenderIdRole:
        if (!event.isNull()) {
            result = event->senderId();
        }
        break;
    case LastEventTimestampRole:
        if (!event.isNull()) {
            result = event->timestamp();
        }
        break;
    case LastEventNewRole:
        if (!event.isNull()) {
            result = event->newEvent();
        }
        break;
    case LastEventTextMessageRole:
        if (!textEvent.isNull()) {
            result = textEvent->message();
        }
        break;
    case LastEventTextMessageTypeRole:
        if (!textEvent.isNull()) {
            result = (int) textEvent->messageType();
        }
        break;
    case LastEventTextMessageFlagsRole:
        if (!textEvent.isNull()) {
            result = (int) textEvent->messageFlags();
        }
        break;
    case LastEventTextReadTimestampRole:
        if (!textEvent.isNull()) {
            result = textEvent->readTimestamp();
        }
        break;
    case LastEventCallMissedRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent->missed();
        }
        break;
    case LastEventCallDurationRole:
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

    History::Threads threads = mThreadView->nextPage();
    qDebug() << "Got threads:" << threads.count();
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
    // remove all events from the model
    beginRemoveRows(QModelIndex(), 0, mThreads.count() - 1);
    mThreads.clear();
    endRemoveRows();

    // and fetch again
    mCanFetchMore = true;

    History::FilterPtr queryFilter;
    History::SortPtr querySort;

    if (!mThreadView.isNull()) {
        mThreadView->disconnect(this);
    }

    if (mFilter) {
        queryFilter = mFilter->filter();
    }
    mThreadView = History::Manager::instance()->queryThreads((History::EventType)mType, querySort, queryFilter);
    connect(mThreadView.data(),
            SIGNAL(threadsAdded(History::Threads)),
            SLOT(onThreadsAdded(History::Threads)));
    connect(mThreadView.data(),
            SIGNAL(threadsModified(History::Threads)),
            SLOT(onThreadsModified(History::Threads)));
    connect(mThreadView.data(),
            SIGNAL(threadsRemoved(History::Threads)),
            SLOT(onThreadsRemoved(History::Threads)));

    fetchMore(QModelIndex());
}

void HistoryThreadModel::onThreadsAdded(const History::Threads &threads)
{
    if (threads.isEmpty()) {
        return;
    }

    // FIXME: handle sorting
    beginInsertRows(QModelIndex(), mThreads.count(), mThreads.count() + threads.count() - 1);
    mThreads << threads;
    endInsertRows();
}

void HistoryThreadModel::onThreadsModified(const History::Threads &threads)
{
    // FIXME: implement
}

void HistoryThreadModel::onThreadsRemoved(const History::Threads &threads)
{
    // FIXME: implement
}
