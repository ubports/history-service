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

#include "historyeventmodel.h"
#include "eventview.h"
#include "historyqmltexteventattachment.h"
#include "manager.h"
#include "contactmatcher_p.h"
#include <QDBusMetaType>
#include <QDebug>
#include <QTimerEvent>

HistoryEventModel::HistoryEventModel(QObject *parent) :
    HistoryModel(parent), mCanFetchMore(true), mEventWritingTimer(0)
{
    // configure the roles
    mRoles = HistoryModel::roleNames();
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
    mRoles[RemoteParticipantRole] = "remoteParticipant";
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

    QVariant result = eventData(mEvents[index.row()], role);
    if (result.isNull()) {
        result = HistoryModel::data(index, role);
    }
    return result;
 }

QVariant HistoryEventModel::eventData(const History::Event &event, int role) const
{
    History::TextEvent textEvent;
    History::VoiceEvent voiceEvent;

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
    case EventIdRole:
        result = event.eventId();
        break;
    case SenderIdRole:
        result = ContactMatcher::instance()->contactInfo(event.accountId(), event.senderId());
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
    case PropertiesRole:
        result = event.properties();
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
    case RemoteParticipantRole:
        if (!voiceEvent.isNull()) {
            result = voiceEvent.remoteParticipant();
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

    if (events.isEmpty()) {
        mCanFetchMore = false;
        Q_EMIT canFetchMoreChanged();
    } else {
        Q_FOREACH(const History::Event &event, events) {
            // watch for contact changes for the given identifiers
            Q_FOREACH(const History::Participant &participant, event.participants()) {
                watchContactInfo(event.accountId(), participant.identifier(), participant.properties());
            }
        }

        beginInsertRows(QModelIndex(), mEvents.count(), mEvents.count() + events.count() - 1);
        mEvents << events;
        endInsertRows();
    }
}

QHash<int, QByteArray> HistoryEventModel::roleNames() const
{
    return mRoles;
}

bool HistoryEventModel::removeEvents(const QVariantList &eventsProperties)
{
    History::Events events;
    Q_FOREACH(const QVariant &entry, eventsProperties) {
        QVariantMap eventProperties = entry.toMap();
        History::Event event;
        switch (eventProperties[History::FieldType].toInt()) {
        case History::EventTypeText:
            event = History::TextEvent::fromProperties(eventProperties);
            break;
        case History::EventTypeVoice:
            event = History::VoiceEvent::fromProperties(eventProperties);
            break;
        }

        if (!event.isNull()) {
            events << event;
        }
    }

    if (events.isEmpty()) {
        return false;
    }

    return History::Manager::instance()->removeEvents(events);
}

bool HistoryEventModel::writeEvents(const QVariantList &eventsProperties)
{
    History::Events events;
    Q_FOREACH(const QVariant &entry, eventsProperties) {
        QVariantMap eventProperties = entry.toMap();
        History::Event event;
        switch (eventProperties[History::FieldType].toInt()) {
        case History::EventTypeText:
            event = History::TextEvent::fromProperties(eventProperties);
            break;
        case History::EventTypeVoice:
            event = History::VoiceEvent::fromProperties(eventProperties);
            break;
        }

        if (!event.isNull()) {
            events << event;
        }
    }

    if (events.isEmpty()) {
        return false;
    }

    return History::Manager::instance()->writeEvents(events);
}

bool HistoryEventModel::removeEventAttachment(const QString &accountId, const QString &threadId, const QString &eventId, int eventType, const QString &attachmentId)
{
    History::TextEvent textEvent;
    History::Event event = History::Manager::instance()->getSingleEvent((History::EventType)eventType, accountId, threadId, eventId);
    if (event.type() != History::EventTypeText) {
        qWarning() << "Trying to remove an attachment from a non text event";
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
        qWarning() << "No attachment found for id " << attachmentId;
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

    Q_FOREACH(const History::Event &event, events) {
        // if the event is already on the model, skip it
        if (mEvents.contains(event)) {
            continue;
        }

        int pos = positionForItem(event.properties());
        beginInsertRows(QModelIndex(), pos, pos);
        mEvents.insert(pos, event);
        endInsertRows();
    }
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
    HistoryModel::timerEvent(event);
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
    }
}

History::Events HistoryEventModel::fetchNextPage()
{
    return mView->nextPage();
}
