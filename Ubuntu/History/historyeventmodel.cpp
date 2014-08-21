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
#include "historyqmlsort.h"
#include "eventview.h"
#include "intersectionfilter.h"
#include "manager.h"
#include "thread.h"
#include "textevent.h"
#include "texteventattachment.h"
#include "historyqmltexteventattachment.h"
#include "thread.h"
#include "voiceevent.h"
#include <QDebug>
#include <QTimerEvent>
#include <QDBusMetaType>

HistoryEventModel::HistoryEventModel(QObject *parent) :
    QAbstractListModel(parent), mCanFetchMore(true), mFilter(0),
    mSort(new HistoryQmlSort(this)), mType(HistoryThreadModel::EventTypeText),
    mEventWritingTimer(0), mUpdateTimer(0)
{
    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), this, SIGNAL(countChanged()));

    // configure the roles
    mRoles[AccountIdRole] = "accountId";
    mRoles[ThreadIdRole] = "threadId";
    mRoles[ParticipantsRole] = "participants";
    mRoles[TypeRole] = "type";
    mRoles[EventIdRole] = "eventId";
    mRoles[SenderIdRole] = "senderId";
    mRoles[TimestampRole] = "timestamp";
    mRoles[DateRole] = "date";
    mRoles[NewEventRole] = "newEvent";
    mRoles[TextMessageRole] = "textMessage";
    mRoles[TextMessageTypeRole] = "textMessageType";
    mRoles[TextMessageStatusRole] = "textMessageStatus";
    mRoles[TextMessageAttachmentsRole] = "textMessageAttachments";
    mRoles[TextReadTimestampRole] = "textReadTimestamp";
    mRoles[TextReadSubjectRole] = "textSubject";
    mRoles[CallMissedRole] = "callMissed";
    mRoles[CallDurationRole] = "callDuration";

    // create the view and get some objects
    triggerQueryUpdate();
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

    return eventData(mEvents[index.row()], role);
}

QVariant HistoryEventModel::eventData(const History::Event &event, int role) const
{
    History::TextEvent textEvent;
    History::VoiceEvent voiceEvent;
    History::Thread thread;

    switch (event.type()) {
    case History::EventTypeText:
        textEvent = event;
        break;
    case History::EventTypeVoice:
        voiceEvent = event;
        break;
    }

    QVariant result;

    switch (role) {
    case AccountIdRole:
        result = event.accountId();
        break;
    case ThreadIdRole:
        result = event.threadId();
        break;
    case ParticipantsRole:
        result = event.participants();
        break;
    case TypeRole:
        result = event.type();
        break;
    case EventIdRole:
        result = event.eventId();
        break;
    case SenderIdRole:
        result = event.senderId();
        break;
    case TimestampRole:
        result = event.timestamp();
        break;
    case DateRole:
        result = event.timestamp().date();
        break;
    case NewEventRole:
        result = event.newEvent();
        break;
    case TextMessageRole:
        if (!textEvent.isNull()) {
            result = textEvent.message();
        }
        break;
    case TextMessageTypeRole:
        if (!textEvent.isNull()) {
            result = (int)textEvent.messageType();
        }
        break;
    case TextMessageStatusRole:
        if (!textEvent.isNull()) {
            result = (int)textEvent.messageStatus();
        }
        break;
    case TextReadTimestampRole:
        if (!textEvent.isNull()) {
            result = textEvent.readTimestamp();
        }
        break;
    case TextReadSubjectRole:
        if (!textEvent.isNull()) {
            result = textEvent.subject();
        }
        break;
    case TextMessageAttachmentsRole:
        if (!textEvent.isNull()) {
            if (mAttachmentCache.contains(textEvent)) {
                result = mAttachmentCache.value(textEvent);
            } else {
                QList<QVariant> attachments;
                Q_FOREACH(const History::TextEventAttachment &attachment, textEvent.attachments()) {
                    attachments << QVariant::fromValue(new HistoryQmlTextEventAttachment(attachment, const_cast<HistoryEventModel*>(this)));
                }
                mAttachmentCache[textEvent] = attachments;
                result = attachments;
            }
        }
        break;
    case CallMissedRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent.missed();
        }
        break;
    case CallDurationRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent.duration();
        }
        break;
    }

    return result;
}

bool HistoryEventModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid() || !mFilter || mView.isNull()) {
        return false;
    }

    return mCanFetchMore;
}

void HistoryEventModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid() || !mFilter || mView.isNull()) {
        return;
    }

    History::Events events = fetchNextPage();

    qDebug() << "Got events:" << events.count();
    if (events.isEmpty()) {
        mCanFetchMore = false;
        Q_EMIT canFetchMoreChanged();
    } else {
        beginInsertRows(QModelIndex(), mEvents.count(), mEvents.count() + events.count() - 1);
        mEvents << events;
        endInsertRows();
    }
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
                SLOT(triggerQueryUpdate()));
    }

    Q_EMIT filterChanged();
    triggerQueryUpdate();
}

HistoryQmlSort *HistoryEventModel::sort() const
{
    return mSort;
}

void HistoryEventModel::setSort(HistoryQmlSort *value)
{
    // disconnect the previous sort
    if (mSort) {
        mSort->disconnect(this);
    }

    mSort = value;
    if (mSort) {
        connect(mSort,
                SIGNAL(sortChanged()),
                SLOT(triggerQueryUpdate()));
    }

    Q_EMIT sortChanged();
    triggerQueryUpdate();
}

HistoryThreadModel::EventType HistoryEventModel::type() const
{
    return mType;
}

void HistoryEventModel::setType(HistoryThreadModel::EventType value)
{
    mType = value;
    Q_EMIT typeChanged();
    triggerQueryUpdate();
}

QString HistoryEventModel::threadIdForParticipants(const QString &accountId, int eventType, const QStringList &participants, int matchFlags, bool create)
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

bool HistoryEventModel::removeEvent(const QString &accountId, const QString &threadId, const QString &eventId, int eventType)
{
    History::Event event = History::Manager::instance()->getSingleEvent((History::EventType)eventType, accountId, threadId, eventId);
    return History::Manager::instance()->removeEvents(History::Events() << event);
}

bool HistoryEventModel::removeEventAttachment(const QString &accountId, const QString &threadId, const QString &eventId, int eventType, const QString &attachmentId)
{
    History::TextEvent textEvent;
    History::Event event = History::Manager::instance()->getSingleEvent((History::EventType)eventType, accountId, threadId, eventId);
    if (event.type() != History::EventTypeText) {
        qDebug() << "Trying to remove an attachment from a non text event";
        return false;
    }
    QVariantMap properties = event.properties();
    QList<QVariantMap> attachmentProperties = qdbus_cast<QList<QVariantMap> >(properties[History::FieldAttachments]);
    QList<QVariantMap> newAttachmentProperties;
    int count = 0;
    Q_FOREACH(const QVariantMap &map, attachmentProperties) {
        if (map[History::FieldAttachmentId] != attachmentId) {
            count++;
            newAttachmentProperties << map;
        }
    }
    if (count == attachmentProperties.size()) {
        qDebug() << "No attachment found for id " << attachmentId;
        return false;
    }
    properties[History::FieldAttachments] = QVariant::fromValue(newAttachmentProperties);
    textEvent = History::TextEvent::fromProperties(properties);

    return History::Manager::instance()->writeEvents(History::Events() << textEvent);
}

bool HistoryEventModel::markEventAsRead(const QString &accountId, const QString &threadId, const QString &eventId, int eventType)
{
    History::Event event = History::Manager::instance()->getSingleEvent((History::EventType)eventType, accountId, threadId, eventId);
    event.setNewEvent(false);
    if (event.type() == History::EventTypeText) {
        History::TextEvent textEvent = event;
        textEvent.setReadTimestamp(QDateTime::currentDateTime());
        event = textEvent;
    }
    mEventWritingQueue << event;
    if (mEventWritingTimer != 0) {
        killTimer(mEventWritingTimer);
    }
    mEventWritingTimer  = startTimer(500);
    return true;
}

void HistoryEventModel::updateQuery()
{
    // remove all events from the model
    if (!mEvents.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, mEvents.count() - 1);
        mEvents.clear();
        endRemoveRows();
    }

    // and create the view again
    History::Filter queryFilter;
    History::Sort querySort;

    if (!mView.isNull()) {
        mView->disconnect(this);
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
    connect(mView.data(),
            SIGNAL(invalidated()),
            SLOT(triggerQueryUpdate()));

    mCanFetchMore = true;
    Q_EMIT canFetchMoreChanged();

    Q_FOREACH(const QVariant &attachment, mAttachmentCache) {
        HistoryQmlTextEventAttachment *qmlAttachment = attachment.value<HistoryQmlTextEventAttachment *>();
        if(qmlAttachment) {
            qmlAttachment->deleteLater();
        }
    }
    mAttachmentCache.clear();

    fetchMore(QModelIndex());
}

void HistoryEventModel::onEventsAdded(const History::Events &events)
{
    if (!events.count()) {
        return;
    }

    // filter the list for items already in the model
    History::Events filteredEvents;
    Q_FOREACH(const History::Event &event, events) {
        if (!mEvents.contains(event)) {
            filteredEvents << event;
        }
    }

    //FIXME: handle sorting
    beginInsertRows(QModelIndex(), mEvents.count(), mEvents.count() + filteredEvents.count() - 1);
    mEvents << filteredEvents;
    endInsertRows();
}

void HistoryEventModel::onEventsModified(const History::Events &events)
{
    History::Events newEvents;
    Q_FOREACH(const History::Event &event, events) {
        int pos = mEvents.indexOf(event);
        if (pos >= 0) {
            mEvents[pos] = event;
            QModelIndex idx = index(pos);
            if (event.type() == History::EventTypeText) {
                History::TextEvent textEvent = event;
                mAttachmentCache.remove(textEvent);
            }
            Q_EMIT dataChanged(idx, idx);
        } else {
            newEvents << event;
        }
    }

    // append the events that were not yet on the model
    if (!newEvents.isEmpty()) {
        onEventsAdded(newEvents);
    }
}

void HistoryEventModel::onEventsRemoved(const History::Events &events)
{
    Q_FOREACH(const History::Event &event, events) {
        int pos = mEvents.indexOf(event);
        if (pos >= 0) {
            beginRemoveRows(QModelIndex(), pos, pos);
            mEvents.removeAt(pos);
            endRemoveRows();
        }
    }

    // FIXME: there is a corner case here: if an event was not loaded yet, but was already
    // removed by another client, it will still show up when a new page is requested. Maybe it
    // should be handle internally in History::EventView?
}

void HistoryEventModel::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mEventWritingTimer) {
        killTimer(mEventWritingTimer);
        mEventWritingTimer = 0;

        if (mEventWritingQueue.isEmpty()) {
            return;
        }

        qDebug() << "Goint to update" << mEventWritingQueue.count() << "events.";
        if (History::Manager::instance()->writeEvents(mEventWritingQueue)) {
            qDebug() << "... succeeded!";
            mEventWritingQueue.clear();
        }
    } else if (event->timerId() == mUpdateTimer) {
        killTimer(mUpdateTimer);
        mUpdateTimer = 0;
        updateQuery();
    }
}

History::Events HistoryEventModel::fetchNextPage()
{
    return mView->nextPage();
}

QVariant HistoryEventModel::get(int row) const
{
    if (row >= this->rowCount() || row < 0) {
        return QVariant();
    }

    return QVariant(mEvents[row].properties());
}

void HistoryEventModel::triggerQueryUpdate()
{
    if (mUpdateTimer) {
        killTimer(mUpdateTimer);
    }
    // delay the loading of the model data until the settings settle down
    mUpdateTimer = startTimer(100);
}
