/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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
#include "manager.h"
#include <QTimerEvent>

HistoryModel::HistoryModel(QObject *parent) :
    QAbstractListModel(parent), mFilter(0), mSort(new HistoryQmlSort(this)),
    mType(EventTypeText), mMatchContacts(false), mUpdateTimer(0)
{
    // configure the roles
    mRoles[AccountIdRole] = "accountId";
    mRoles[ThreadIdRole] = "threadId";
    mRoles[ParticipantsRole] = "participants";
    mRoles[TypeRole] = "type";
    mRoles[PropertiesRole] = "properties";

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), this, SIGNAL(countChanged()));
    connect(ContactMatcher::instance(),
            SIGNAL(contactInfoChanged(QString,QVariantMap)),
            SLOT(onContactInfoChanged(QString,QVariantMap)));

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
    case ParticipantsRole:
        if (mMatchContacts) {
            result = ContactMatcher::instance()->contactInfo(properties[History::FieldParticipants].toStringList());
        } else {
            result = properties[History::FieldParticipants];
        }
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
    mMatchContacts = value;
    Q_EMIT matchContactsChanged();

    // mark all indexes as changed
    if (rowCount() > 0) {
        Q_EMIT dataChanged(index(0), index(rowCount()-1));
    }
}

QString HistoryModel::threadIdForParticipants(const QString &accountId, int eventType, const QStringList &participants, int matchFlags, bool create)
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

void HistoryModel::onContactInfoChanged(const QString &phoneNumber, const QVariantMap &contactInfo)
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
        QStringList participants = properties[History::FieldParticipants].toStringList();
        Q_FOREACH(const QString &participant, participants) {
            if (PhoneUtils::comparePhoneNumbers(participant, phoneNumber)) {
                changedIndexes << idx;
            }
        }
    }

    // now emit the dataChanged signal to all changed indexes
    Q_FOREACH(const QModelIndex &idx, changedIndexes) {
        Q_EMIT dataChanged(idx, idx);
    }
}

void HistoryModel::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mUpdateTimer) {
        killTimer(mUpdateTimer);
        mUpdateTimer = 0;
        updateQuery();
    }
}


bool HistoryModel::compareParticipants(const QStringList &list1, const QStringList &list2)
{
    if (list1.count() != list2.count()) {
        return false;
    }

    int found = 0;
    Q_FOREACH(const QString &participant, list1) {
        Q_FOREACH(const QString &item, list2) {
            if (PhoneUtils::comparePhoneNumbers(participant, item)) {
                found++;
                break;
            }
        }
    }

    return found == list1.count();
}

bool HistoryModel::lessThan(const QVariantMap &left, const QVariantMap &right) const
{
    QVariant leftValue = left[sort()->sortField()];
    QVariant rightValue = right[sort()->sortField()];
    return leftValue < rightValue;
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
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }

    return index(row).data(PropertiesRole);
}

void HistoryModel::triggerQueryUpdate()
{
    if (mUpdateTimer) {
        killTimer(mUpdateTimer);
    }
    // delay the loading of the model data until the settings settle down
    mUpdateTimer = startTimer(100);
}
