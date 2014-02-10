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
#include "historyqmlsort.h"
#include "manager.h"
#include "threadview.h"
#include "textevent.h"
#include "texteventattachment.h"
#include "historyqmltexteventattachment.h"
#include "voiceevent.h"
#include <QDebug>

Q_DECLARE_METATYPE(History::TextEventAttachments)

HistoryThreadModel::HistoryThreadModel(QObject *parent) :
    QAbstractListModel(parent), mCanFetchMore(true), mFilter(0), mSort(0), mType(EventTypeText)
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
    mRoles[LastEventDateRole] = "eventDate";
    mRoles[LastEventNewRole] = "eventNew";
    mRoles[LastEventTextMessageRole] = "eventTextMessage";
    mRoles[LastEventTextMessageTypeRole] = "eventTextMessageType";
    mRoles[LastEventTextMessageStatusRole] = "eventTextMessageStatus";
    mRoles[LastEventTextReadTimestampRole] = "eventTextReadTimestamp";
    mRoles[LastEventTextAttachmentsRole] = "eventTextAttachments";
    mRoles[LastEventTextSubjectRole] = "eventTextSubject";
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

    History::Thread thread = mThreads[index.row()];
    History::Event event = thread.lastEvent();
    History::TextEvent textEvent;
    History::VoiceEvent voiceEvent;

    if (!event.isNull()) {
        switch (event.type()) {
        case History::EventTypeText:
            textEvent = event;
            break;
        case History::EventTypeVoice:
            voiceEvent = event;
            break;
        }
    }

    QVariant result;
    switch (role) {
    case AccountIdRole:
        result = thread.accountId();
        break;
    case ThreadIdRole:
        result = thread.threadId();
        break;
    case TypeRole:
        result = (int) thread.type();
        break;
    case ParticipantsRole:
        result = thread.participants();
        break;
    case CountRole:
        result = thread.count();
        break;
    case UnreadCountRole:
        result = thread.unreadCount();
        break;
    case LastEventIdRole:
        if (!event.isNull()) {
            result = event.eventId();
        }
        break;
    case LastEventSenderIdRole:
        if (!event.isNull()) {
            result = event.senderId();
        }
        break;
    case LastEventTimestampRole:
        if (!event.isNull()) {
            result = event.timestamp();
        }
        break;
    case LastEventDateRole:
        if (!event.isNull()) {
            result = event.timestamp().date();
        }
        break;
    case LastEventNewRole:
        if (!event.isNull()) {
            result = event.newEvent();
        }
        break;
    case LastEventTextMessageRole:
        if (!textEvent.isNull()) {
            result = textEvent.message();
        }
        break;
    case LastEventTextMessageTypeRole:
        if (!textEvent.isNull()) {
            result = (int) textEvent.messageType();
        }
        break;
    case LastEventTextMessageStatusRole:
        if (!textEvent.isNull()) {
            result = (int) textEvent.messageStatus();
        }
        break;
    case LastEventTextReadTimestampRole:
        if (!textEvent.isNull()) {
            result = textEvent.readTimestamp();
        }
        break;
    case LastEventTextSubjectRole:
        if (!textEvent.isNull()) {
            result = textEvent.subject();
        }
        break;
    case LastEventTextAttachmentsRole:
        if (!textEvent.isNull()) {
            if (mAttachmentCache.contains(textEvent)) {
                result = mAttachmentCache.value(textEvent);
            } else {
                QList<QVariant> attachments;
                Q_FOREACH(const History::TextEventAttachment &attachment, textEvent.attachments()) {
                    attachments << QVariant::fromValue(new HistoryQmlTextEventAttachment(attachment, const_cast<HistoryThreadModel*>(this)));
                }
                mAttachmentCache[textEvent] = attachments;
                result = attachments;
            }
        }
        break;
    case LastEventCallMissedRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent.missed();
        }
        break;
    case LastEventCallDurationRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent.duration();
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
    if (threads.isEmpty()) {
        mCanFetchMore = false;
    } else {
        beginInsertRows(QModelIndex(), mThreads.count(), mThreads.count() + threads.count() - 1);
        mThreads << threads;
        endInsertRows();
    }
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

HistoryQmlSort *HistoryThreadModel::sort() const
{
    return mSort;
}

void HistoryThreadModel::setSort(HistoryQmlSort *value)
{
    // disconnect the previous sort
    if (mSort) {
        mSort->disconnect(this);
    }

    mSort = value;
    if (mSort) {
        connect(mSort,
                SIGNAL(sortChanged()),
                SLOT(updateQuery()));
    }

    Q_EMIT sortChanged();
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

QString HistoryThreadModel::threadIdForParticipants(const QString &accountId, int eventType, const QStringList &participants, int matchFlags, bool create)
{
    if (participants.isEmpty()) {
        return QString::null;
    }

    History::Thread thread = History::Manager::instance()->threadForParticipants(accountId,
                                                                                 (History::EventType)eventType,
                                                                                 participants,
                                                                                 (History::MatchFlags)matchFlags,
                                                                                 create);
    if (!thread.isNull()) {
        return thread.threadId();
    }

    return QString::null;
}

bool HistoryThreadModel::removeThread(const QString &accountId, const QString &threadId, int eventType)
{
    History::Thread thread = History::Manager::instance()->getSingleThread((History::EventType)eventType, accountId, threadId);
    return History::Manager::instance()->removeThreads(History::Threads() << thread);
}

void HistoryThreadModel::updateQuery()
{
    // remove all events from the model
    if (!mThreads.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, mThreads.count() - 1);
        mThreads.clear();
        endRemoveRows();
    }
 
    // and fetch again
    mCanFetchMore = true;

    History::Filter queryFilter;
    History::Sort querySort;

    if (!mThreadView.isNull()) {
        mThreadView->disconnect(this);
    }

    if (mFilter) {
        queryFilter = mFilter->filter();
    }

    if (mSort) {
        querySort = mSort->sort();
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
    connect(mThreadView.data(),
            SIGNAL(invalidated()),
            SLOT(updateQuery()));

    Q_FOREACH(const QVariant &attachment, mAttachmentCache) {
        HistoryQmlTextEventAttachment *qmlAttachment = attachment.value<HistoryQmlTextEventAttachment *>();
        if(qmlAttachment) {
            qmlAttachment->deleteLater();
        }
    }
    mAttachmentCache.clear();

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
    Q_FOREACH(const History::Thread &thread, threads) {
        int pos = mThreads.indexOf(thread);
        if (pos >= 0) {
            mThreads[pos] = thread;
            QModelIndex idx = index(pos);
            Q_EMIT dataChanged(idx, idx);
        }
    }

    // FIXME: append modified threads that are not loaded yet and make sure they don´t
    // get added twice to the model
}

void HistoryThreadModel::onThreadsRemoved(const History::Threads &threads)
{
    Q_FOREACH(const History::Thread &thread, threads) {
        int pos = mThreads.indexOf(thread);
        if (pos >= 0) {
            beginRemoveRows(QModelIndex(), pos, pos);
            mThreads.removeAt(pos);
            endRemoveRows();
        }
    }

    // FIXME: there is a corner case here: if a thread was not loaded yet, but was already
    // removed by another client, it will still show up when a new page is requested. Maybe it
    // should be handle internally in History::ThreadView?
}
