/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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

#include "historymodel.h"
#include "historyqmlfilter.h"
#include "historyqmlsort.h"
#include "contactmatcher_p.h"
#include "phoneutils_p.h"
#include "thread.h"
#include "textevent.h"
#include "manager.h"
#include "utils_p.h"
#include "voiceevent.h"
#include <QTimerEvent>
#include <QCryptographicHash>
#include <QDebug>

HistoryModel::HistoryModel(QObject *parent) :
    QAbstractListModel(parent), mFilter(0), mSort(new HistoryQmlSort(this)),
    mType(EventTypeText), mMatchContacts(false), mUpdateTimer(0), mEventWritingTimer(0), mThreadWritingTimer(0), mWaitingForQml(false)
{
    // configure the roles
    mRoles[AccountIdRole] = "accountId";
    mRoles[ThreadIdRole] = "threadId";
    mRoles[ParticipantsRole] = "participants";
    mRoles[ParticipantsRemotePendingRole] = "remotePendingParticipants";
    mRoles[ParticipantsLocalPendingRole] = "localPendingParticipants";
    mRoles[TypeRole] = "type";
    mRoles[TimestampRole] = "timestamp";
    mRoles[PropertiesRole] = "properties";

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), this, SIGNAL(countChanged()));

    // reset the view when the service is stopped or started
    connect(History::Manager::instance(), SIGNAL(serviceRunningChanged()),
            this, SLOT(triggerQueryUpdate()));

    // create the view and get some objects
    triggerQueryUpdate();
}

bool HistoryModel::canFetchMore(const QModelIndex &parent) const
{
    return false;
}

void HistoryModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    // do nothing, just make the method invokable
}

QHash<int, QByteArray> HistoryModel::roleNames() const
{
    return mRoles;
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }

    QVariantMap properties = index.data(PropertiesRole).toMap();
    QVariant result;
    switch (role) {
    case AccountIdRole:
        result = properties[History::FieldAccountId];
        break;
    case ThreadIdRole:
        result = properties[History::FieldThreadId];
        break;
    case TypeRole:
        result = properties[History::FieldType];
        break;
    case ParticipantsRole: {
        History::Participants participants = History::Participants::fromVariantList(properties[History::FieldParticipants].toList())
                                             .filterByState(History::ParticipantStateRegular);
        if (mMatchContacts) {
            QVariantList finalParticipantsList;
            QVariantList participantsInfo = History::ContactMatcher::instance()->contactInfo(properties[History::FieldAccountId].toString(),
                                                                                             participants.identifiers());
            for (int i = 0; i < participantsInfo.count(); ++i) {
                QVariantMap newMap = participantsInfo[i].toMap();
                History::Participant participant = participants[i];
                newMap[History::FieldParticipantState] = participant.state();
                newMap[History::FieldParticipantRoles] = participant.roles();
                finalParticipantsList << newMap;
            }
            result = finalParticipantsList;
        } else {
            //FIXME: handle contact changes
            result = participants.identifiers();
        }
        break;
    }
    case ParticipantsRemotePendingRole: {
        History::Participants participants = History::Participants::fromVariantList(properties[History::FieldParticipants].toList())
                                             .filterByState(History::ParticipantStateRemotePending);
        if (mMatchContacts) {
            QVariantList finalParticipantsList;
            QVariantList participantsInfo = History::ContactMatcher::instance()->contactInfo(properties[History::FieldAccountId].toString(),
                                                                      participants.identifiers());
            int count = 0;
            Q_FOREACH(const QVariant &participantInfo, participantsInfo) {
                QVariantMap newMap = participantInfo.toMap();
                newMap[History::FieldParticipantState] = participants.at(count).state();
                newMap[History::FieldParticipantRoles] = participants.at(count++).roles();
                finalParticipantsList << newMap;
            }
            result = finalParticipantsList;
        } else {
            //FIXME: handle contact changes
            result = participants.identifiers();
        }

        break;
    }
    case ParticipantsLocalPendingRole: {
        History::Participants participants = History::Participants::fromVariantList(properties[History::FieldParticipants].toList())
                                             .filterByState(History::ParticipantStateLocalPending);
        if (mMatchContacts) {
            QVariantList finalParticipantsList;
            QVariantList participantsInfo = History::ContactMatcher::instance()->contactInfo(properties[History::FieldAccountId].toString(),
                                                                      participants.identifiers());
            int count = 0;
            Q_FOREACH(const QVariant &participantInfo, participantsInfo) {
                QVariantMap newMap = participantInfo.toMap();
                newMap[History::FieldParticipantState] = participants.at(count).state();
                newMap[History::FieldParticipantRoles] = participants.at(count++).roles();
                finalParticipantsList << newMap;
            }
            result = finalParticipantsList;
        } else {
            //FIXME: handle contact changes
            result = participants.identifiers();
        }

        break;
    }
    case ParticipantIdsRole:
        result = History::Participants::fromVariantList(properties[History::FieldParticipants].toList()).identifiers();
        break;
    case TimestampRole:
        result = QDateTime::fromString(properties[History::FieldTimestamp].toString(), Qt::ISODate);
        break;
    }

    return result;
}

HistoryQmlFilter *HistoryModel::filter() const
{
    return mFilter;
}

void HistoryModel::setFilter(HistoryQmlFilter *value)
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

HistoryQmlSort *HistoryModel::sort() const
{
    return mSort;
}

void HistoryModel::setSort(HistoryQmlSort *value)
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

HistoryModel::EventType HistoryModel::type() const
{
    return mType;
}

void HistoryModel::setType(EventType value)
{
    mType = value;
    Q_EMIT typeChanged();
    triggerQueryUpdate();
}

bool HistoryModel::matchContacts() const
{
    return mMatchContacts;
}

void HistoryModel::setMatchContacts(bool value)
{
    if (mMatchContacts == value) {
        return;
    }

    mMatchContacts = value;
    Q_EMIT matchContactsChanged();

    if (mMatchContacts) {
        connect(History::ContactMatcher::instance(),
                SIGNAL(contactInfoChanged(QString,QString,QVariantMap)),
                SLOT(onContactInfoChanged(QString,QString,QVariantMap)));
    } else {
        History::ContactMatcher::instance()->disconnect(this);
    }

    // mark all indexes as changed
    if (rowCount() > 0) {
        Q_EMIT dataChanged(index(0), index(rowCount()-1));
    }
}

QVariantMap HistoryModel::threadForProperties(const QString &accountId, int eventType, const QVariantMap &properties, int matchFlags, bool create)
{
    QVariantMap newProperties = properties;
    if (properties.isEmpty()) {
        return QVariantMap();
    }

    if (newProperties.contains(History::FieldParticipantIds)) {
        newProperties[History::FieldParticipantIds] = newProperties[History::FieldParticipantIds].toStringList();
    }
 
    History::Thread thread = History::Manager::instance()->threadForProperties(accountId,
                                                                               (History::EventType)eventType,
                                                                               newProperties,
                                                                               (History::MatchFlags)matchFlags,
                                                                               create);
    if (!thread.isNull()) {
        return thread.properties();
    }

    return QVariantMap();
}

QString HistoryModel::threadIdForProperties(const QString &accountId, int eventType, const QVariantMap &properties, int matchFlags, bool create)
{
    QVariantMap newProperties = properties;
    if (properties.isEmpty()) {
        return QString::null;
    }

    if (newProperties.contains(History::FieldParticipantIds)) {
        newProperties[History::FieldParticipantIds] = newProperties[History::FieldParticipantIds].toStringList();
    }

    History::Thread thread = History::Manager::instance()->threadForProperties(accountId,
                                                                               (History::EventType)eventType,
                                                                               newProperties,
                                                                               (History::MatchFlags)matchFlags,
                                                                               create);
    if (!thread.isNull()) {
        return thread.threadId();
    }

    return QString::null;
}

QVariantMap HistoryModel::threadForParticipants(const QString &accountId, int eventType, const QStringList &participants, int matchFlags, bool create)
{
    if (participants.isEmpty()) {
        return QVariantMap();
    }

    QVariantMap properties;
    properties[History::FieldParticipantIds] = participants;

    History::Thread thread = History::Manager::instance()->threadForProperties(accountId,
                                                                               (History::EventType)eventType,
                                                                               properties,
                                                                               (History::MatchFlags)matchFlags,
                                                                               create);
    if (!thread.isNull()) {
        return thread.properties();
    }

    return QVariantMap();
}

QString HistoryModel::threadIdForParticipants(const QString &accountId, int eventType, const QStringList &participants, int matchFlags, bool create)
{
    if (participants.isEmpty()) {
        return QString::null;
    }

    QVariantMap properties;
    properties[History::FieldParticipantIds] = participants;

    History::Thread thread = History::Manager::instance()->threadForProperties(accountId,
                                                                               (History::EventType)eventType,
                                                                               properties,
                                                                               (History::MatchFlags)matchFlags,
                                                                               create);
    if (!thread.isNull()) {
        return thread.threadId();
    }

    return QString::null;
}

bool HistoryModel::writeTextInformationEvent(const QString &accountId, const QString &threadId, const QStringList &participants, const QString &message, int informationType, const QString &subject)
{
    if (participants.isEmpty() || threadId.isEmpty() || accountId.isEmpty()) {
        return false;
    }

    History::TextEvent historyEvent = History::TextEvent(accountId,
                                                         threadId,
                                                         QString(QCryptographicHash::hash(QByteArray(
                                                                 QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmsszzz").toLatin1()), 
                                                                 QCryptographicHash::Md5).toHex()),
                                                         "self",
                                                         QDateTime::currentDateTime(),
                                                         false,
                                                         message,
                                                         History::MessageTypeInformation,
                                                         History::MessageStatusUnknown,
                                                         QDateTime::currentDateTime(),
                                                         subject,
                                                         (History::InformationType)informationType);
    History::Events events;
    events << historyEvent;
    return History::Manager::instance()->writeEvents(events);
}

void HistoryModel::onContactInfoChanged(const QString &accountId, const QString &identifier, const QVariantMap &contactInfo)
{
    Q_UNUSED(contactInfo)
    if (!mMatchContacts) {
        return;
    }

    QList<QModelIndex> changedIndexes;
    int count = rowCount();
    for (int i = 0; i < count; ++i) {
        // WARNING: do not use mEvents directly to verify which indexes to change as there is the
        // HistoryGroupedEventsModel which is based on this model and handles the items in a different way
        QModelIndex idx = index(i);
        QVariantMap properties = idx.data(PropertiesRole).toMap();
        History::Participants participants = History::Participants::fromVariantList(properties[History::FieldParticipants].toList());
        Q_FOREACH(const History::Participant &participant, participants) {
            // FIXME: right now we might be grouping threads from different accounts, so we are not enforcing
            // the accountId to be the same as the one from the contact info, but maybe we need to do that
            // in the future?
            if (History::Utils::compareIds(accountId, History::ContactMatcher::normalizeId(participant.identifier()), identifier)) {
                changedIndexes << idx;
            }
        }
    }

    // now emit the dataChanged signal to all changed indexes
    Q_FOREACH(const QModelIndex &idx, changedIndexes) {
        Q_EMIT dataChanged(idx, idx);
    }
}

void HistoryModel::watchContactInfo(const QString &accountId, const QString &identifier, const QVariantMap &currentInfo)
{
    if (mMatchContacts) {
        History::ContactMatcher::instance()->watchIdentifier(accountId, identifier, currentInfo);
    }
}

void HistoryModel::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mUpdateTimer) {
        if (!mWaitingForQml) {
            killTimer(mUpdateTimer);
            mUpdateTimer = 0;
            updateQuery();
        }
    } else if (event->timerId() == mEventWritingTimer) {
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
    } else if (event->timerId() == mThreadWritingTimer) {
        killTimer(mThreadWritingTimer);
        mThreadWritingTimer = 0;

        if (mThreadWritingQueue.isEmpty()) {
            return;
        }

        History::Manager::instance()->markThreadsAsRead(mThreadWritingQueue);
        mThreadWritingQueue.clear();
    }
}

bool HistoryModel::lessThan(const QVariantMap &left, const QVariantMap &right) const
{
    QStringList leftFields = sort()->sortField().split(",");
    QStringList rightFields = sort()->sortField().split(",");

    while(!leftFields.isEmpty()) {
        QVariant leftValue = left[leftFields.takeFirst().trimmed()];
        QVariant rightValue = right[rightFields.takeFirst().trimmed()];

        if (leftValue != rightValue) {
            return leftValue < rightValue;
        }
    }
    return false;
}

int HistoryModel::positionForItem(const QVariantMap &item) const
{
    // do a binary search for the item position on the list
    int lowerBound = 0;
    int upperBound = rowCount() - 1;
    if (upperBound < 0) {
        return 0;
    }

    while (true) {
        int pos = (upperBound + lowerBound) / 2;
        const QVariantMap posItem = index(pos).data(PropertiesRole).toMap();
        if (lowerBound == pos) {
            if (isAscending() ? lessThan(item, posItem) : lessThan(posItem, item)) {
                return pos;
            }
        }
        if (isAscending() ? lessThan(posItem, item) : lessThan(item, posItem)) {
            lowerBound = pos + 1;          // its in the upper
            if (lowerBound > upperBound) {
                return pos += 1;
            }
        } else if (lowerBound > upperBound) {
            return pos;
        } else {
            upperBound = pos - 1;          // its in the lower
        }
    }
}

bool HistoryModel::isAscending() const
{
    return mSort && mSort->sort().sortOrder() == Qt::AscendingOrder;
}

QVariant HistoryModel::get(int row) const
{
    QVariantMap data;
    QModelIndex idx = index(row, 0);
    if (idx.isValid()) {
        QHash<int, QByteArray> roles = roleNames();
        Q_FOREACH(int role, roles.keys()) {
            data.insert(roles[role], idx.data(role));
        }
    }

    return data;
}

bool HistoryModel::markEventAsRead(const QVariantMap &eventProperties)
{
    History::Event event;
    History::EventType type = (History::EventType) eventProperties[History::FieldType].toInt();
    switch (type) {
    case History::EventTypeText:
        event = History::TextEvent::fromProperties(eventProperties);
        break;
    case History::EventTypeVoice:
        event = History::VoiceEvent::fromProperties(eventProperties);
        break;
    }

    event.setNewEvent(false);
    if (event.type() == History::EventTypeText) {
        History::TextEvent textEvent = event;
        textEvent.setReadTimestamp(QDateTime::currentDateTime());
        event = textEvent;
    }
    // for repeated events, keep the last called one only
    if (mEventWritingQueue.contains(event)) {
        mEventWritingQueue.removeOne(event);
    }
    mEventWritingQueue << event;
    if (mEventWritingTimer != 0) {
        killTimer(mEventWritingTimer);
    }
    mEventWritingTimer = startTimer(500);
    return true;
}

void HistoryModel::markThreadsAsRead(const QVariantList &threadsProperties)
{
    Q_FOREACH(const QVariant &entry, threadsProperties) {
        QVariantMap threadProperties = entry.toMap();
        History::Thread thread = History::Thread::fromProperties(threadProperties);

        if (!thread.isNull()) {
            if (mThreadWritingQueue.contains(thread)) {
                mThreadWritingQueue.removeOne(thread);
            }
            mThreadWritingQueue << thread;
        }
    }

    if (mThreadWritingTimer != 0) {
        killTimer(mThreadWritingTimer);
    }
    mThreadWritingTimer = startTimer(500);
}

void HistoryModel::classBegin()
{
    mWaitingForQml = true;
}

void HistoryModel::componentComplete()
{
    mWaitingForQml = false;
    if (mUpdateTimer) {
        killTimer(mUpdateTimer);
        mUpdateTimer = 0;
    }
    updateQuery();
}

void HistoryModel::triggerQueryUpdate()
{
    if (mUpdateTimer) {
        killTimer(mUpdateTimer);
    }

    // delay the loading of the model data until the settings settle down
    mUpdateTimer = startTimer(100);
}
