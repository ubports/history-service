/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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
#include "historyqmltexteventattachment.h"
#include "manager.h"
#include "threadview.h"
#include "voiceevent.h"
#include <QDBusMetaType>

Q_DECLARE_METATYPE(History::TextEventAttachments)
Q_DECLARE_METATYPE(QList<QVariantMap>)

HistoryThreadModel::HistoryThreadModel(QObject *parent) :
    HistoryModel(parent), mCanFetchMore(true)
{
    qRegisterMetaType<QList<QVariantMap> >();
    qDBusRegisterMetaType<QList<QVariantMap> >();
    // configure the roles
    mRoles = HistoryModel::roleNames();
    mRoles[CountRole] = "count";
    mRoles[UnreadCountRole] = "unreadCount";
    mRoles[GroupedThreadsRole] = "groupedThreads";

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
    QVariant result = threadData(thread, role);
    if (result.isNull()) {
        result = HistoryModel::data(index, role);
    }

    return result;
}

QVariant HistoryThreadModel::threadData(const History::Thread &thread, int role) const
{
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
    case CountRole:
        result = thread.count();
        break;
    case UnreadCountRole:
        result = thread.unreadCount();
        break;
    case GroupedThreadsRole:
    {
         QVariantList threads;
         Q_FOREACH(const QVariantMap &thread, thread.groupedThreads()) {
             threads << QVariant::fromValue(thread);
         }
         result = threads;
         break;
    }
    case PropertiesRole:
        result = thread.properties();
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
    if (parent.isValid() || !mFilter || mThreadView.isNull()) {
        return false;
    }

    return mCanFetchMore;
}

void HistoryThreadModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid() || mThreadView.isNull()) {
        return;
    }

    History::Threads threads = fetchNextPage();
    if (threads.isEmpty()) {
        mCanFetchMore = false;
        Q_EMIT canFetchMoreChanged();
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

bool HistoryThreadModel::removeThreads(const QVariantList &threadsProperties)
{
    History::Threads threads;
    Q_FOREACH(const QVariant &entry, threadsProperties) {
        QVariantMap threadProperties = entry.toMap();
        History::Thread thread = History::Thread::fromProperties(threadProperties);

        if (!thread.isNull()) {
            threads << thread;
        }
    }

    if (threads.isEmpty()) {
        return false;
    }

    return History::Manager::instance()->removeThreads(threads);
}

void HistoryThreadModel::updateQuery()
{
    // remove all events from the model
    if (!mThreads.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, mThreads.count() - 1);
        mThreads.clear();
        endRemoveRows();
    }

    History::Filter queryFilter;
    History::Sort querySort;

    if (!mThreadView.isNull()) {
        mThreadView->disconnect(this);
    }

    if (mFilter) {
        queryFilter = mFilter->filter();
    } else {
        // we should not return anything if there is no filter
        return;
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
            SLOT(triggerQueryUpdate()));

    Q_FOREACH(const QVariant &attachment, mAttachmentCache) {
        HistoryQmlTextEventAttachment *qmlAttachment = attachment.value<HistoryQmlTextEventAttachment *>();
        if(qmlAttachment) {
            qmlAttachment->deleteLater();
        }
    }
    mAttachmentCache.clear();

    // and fetch again
    mCanFetchMore = true;
    Q_EMIT canFetchMoreChanged();
    fetchMore(QModelIndex());
}

void HistoryThreadModel::onThreadsAdded(const History::Threads &threads)
{
    if (threads.isEmpty()) {
        return;
    }

    Q_FOREACH(const History::Thread &thread, threads) {
        // if the thread is already inserted, skip it
        if (mThreads.contains(thread)) {
            continue;
        }

        int pos = positionForItem(thread.properties());
        beginInsertRows(QModelIndex(), pos, pos);
        mThreads.insert(pos, thread);
        endInsertRows();
    }
}

void HistoryThreadModel::onThreadsModified(const History::Threads &threads)
{
    History::Threads newThreads;
    Q_FOREACH(const History::Thread &thread, threads) {
        int pos = mThreads.indexOf(thread);
        if (pos >= 0) {
            mThreads[pos] = thread;
            QModelIndex idx = index(pos);
            Q_EMIT dataChanged(idx, idx);
        } else {
            newThreads << thread;
        }
    }

    // add threads that were not yet on the model
    if (!newThreads.isEmpty()) {
        onThreadsAdded(newThreads);
    }
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

History::Threads HistoryThreadModel::fetchNextPage()
{
    return mThreadView->nextPage();
}
