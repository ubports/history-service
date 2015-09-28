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

#include "historygroupedthreadsmodel.h"
#include "utils_p.h"
#include "manager.h"
#include "phoneutils_p.h"
#include <QTimer>
#include <QDebug>
#include <QDBusMetaType>

HistoryGroupedThreadsModel::HistoryGroupedThreadsModel(QObject *parent) :
    HistoryThreadModel(parent)
{
    qDBusRegisterMetaType<QList<QVariantMap> >();
    qRegisterMetaType<QList<QVariantMap> >();
    mGroupThreads = true;
    mRoles = HistoryThreadModel::roleNames();
    mRoles[ThreadsRole] = "threads";
}

QVariant HistoryGroupedThreadsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const HistoryThreadGroup &group = mGroups[index.row()];

    // get the data from the latest thread, and overwrite if necessary
    QVariant result = threadData(group.displayedThread, role);
    switch (role) {
        case HistoryThreadModel::CountRole: {
            int count = 0;
            Q_FOREACH(const History::Thread &thread, group.threads) {
                count += thread.count();
            }
            result = count;
            break;
        }
        case HistoryThreadModel::UnreadCountRole: {
            int count = 0;
            Q_FOREACH(const History::Thread &thread, group.threads) {
                count += thread.unreadCount();
            }
            result = count;
            break;
        }
        case ThreadsRole: {
            QVariantList threads;
            Q_FOREACH(const History::Thread &thread, group.threads) {
                threads << thread.properties();
            }
            result = threads;
            break;
        }
    }

    if (result.isNull()) {
        // get the shared roles
        result = HistoryModel::data(index, role);
    }

    return result;
}

void HistoryGroupedThreadsModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent)) {
        return;
    }

    const History::Threads &threads = fetchNextPage();
    Q_FOREACH(const History::Thread &thread, threads) {
        processThreadGrouping(thread);

        // insert the identifiers in the contact map
        Q_FOREACH(const History::Participant &participant, thread.participants()) {
            watchContactInfo(thread.accountId(), participant.identifier(), participant.properties());
        }
    }
    notifyDataChanged();

    if (threads.isEmpty()) {
        mCanFetchMore = false;
        Q_EMIT canFetchMoreChanged();
    }
}

QHash<int, QByteArray> HistoryGroupedThreadsModel::roleNames() const
{
    return mRoles;
}

QVariant HistoryGroupedThreadsModel::get(int row) const
{
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }

    return data(index(row), ThreadsRole);
}

int HistoryGroupedThreadsModel::existingPositionForEntry(const QVariantMap &properties) const
{
    int pos = -1;
    if (mGroupingProperty == History::FieldParticipants) {
        for (int i = 0; i < mGroups.count(); ++i) {
            const HistoryThreadGroup &group = mGroups[i];
            Q_FOREACH(const History::Thread &groupedThread, group.threads) {
                if (groupedThread.accountId() == properties[History::FieldAccountId].toString() && 
                    groupedThread.threadId() == properties[History::FieldThreadId].toString()) {
                    return i;
                }
            }
        }
        return pos;
    }

    for (int i = 0; i < mGroups.count(); ++i) {
        const HistoryThreadGroup &group = mGroups[i];
        if (properties[mGroupingProperty] == group.displayedThread.properties()[mGroupingProperty]) {
            pos = i;
            break;
        }
    }

    return pos;
}

void HistoryGroupedThreadsModel::removeGroup(const HistoryThreadGroup &group)
{
    int pos = mGroups.indexOf(group);
    if (pos >= 0){
        beginRemoveRows(QModelIndex(), pos, pos);
        mGroups.removeAt(pos);
        endRemoveRows();
    }
}

void HistoryGroupedThreadsModel::updateDisplayedThread(HistoryThreadGroup &group)
{
    int pos = mGroups.indexOf(group);
    if (pos < 0) {
        qWarning() << "Group not found!!";
        return;
    }

    History::Thread displayedThread = group.threads.first();
    QVariantMap displayedProperties = displayedThread.properties();
    Q_FOREACH(const History::Thread &other, group.threads) {
        if (isAscending() ? lessThan(other.properties(), displayedProperties) :
                            lessThan(displayedProperties, other.properties())) {
            displayedThread = other;
            displayedProperties = displayedThread.properties();
        }
    }

    // check if we need to update the order
    int newPos = positionForItem(displayedProperties);

    // NOTE: only set the new displayedThread AFTER calling positionForItem
    group.displayedThread = displayedThread;

    // the positionForItem function might return the pos+1 value for the current item as it considers
    // this to be the position for a new insertion
    if (newPos != pos && newPos != pos+1) {
        beginMoveRows(QModelIndex(), pos, pos, QModelIndex(), newPos);
        // QList::move() behaves in a different way than the QAbstractItemModel moving functions
        // that's why the delta was added
        mGroups.move(pos, newPos > pos ? newPos-1 : newPos);
        endMoveRows();
    }
}

void HistoryGroupedThreadsModel::updateQuery()
{
    // remove all entries and call the query update
    if (!mGroups.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        mGroups.clear();
        endRemoveRows();
    }

    HistoryThreadModel::updateQuery();
}

void HistoryGroupedThreadsModel::onThreadsAdded(const History::Threads &threads)
{
    Q_FOREACH(const History::Thread &thread, threads) {
        processThreadGrouping(thread);
    }

    notifyDataChanged();
}

void HistoryGroupedThreadsModel::onThreadsModified(const History::Threads &threads)
{
    Q_FOREACH(const History::Thread &thread, threads) {
        processThreadGrouping(thread);
    }

    notifyDataChanged();
}

void HistoryGroupedThreadsModel::onThreadsRemoved(const History::Threads &threads)
{
    Q_FOREACH(const History::Thread &thread, threads) {
        removeThreadFromGroup(thread);
    }

    notifyDataChanged();
}

void HistoryGroupedThreadsModel::processThreadGrouping(const History::Thread &thread)
{
    QVariantMap properties = thread.properties();
    int pos = existingPositionForEntry(properties);
    QVariantMap queryProperties;
    queryProperties["groupingProperty"] = mGroupingProperty;
    History::Thread groupedThread = History::Manager::instance()->getSingleThread((History::EventType)mType, thread.accountId(), thread.threadId(), queryProperties);

    // if the group is empty, we need to insert it into the map
    if (pos < 0) {
        HistoryThreadGroup group;
        int newPos = positionForItem(groupedThread.properties());
        group.threads = groupedThread.groupedThreads();
        group.displayedThread = groupedThread;
        beginInsertRows(QModelIndex(), newPos, newPos);
        mGroups.insert(newPos, group);
        endInsertRows();
        return;
    }

    HistoryThreadGroup &group = mGroups[pos];
    group.threads = groupedThread.groupedThreads();

    updateDisplayedThread(group);
    markGroupAsChanged(group);
}

void HistoryGroupedThreadsModel::removeThreadFromGroup(const History::Thread &thread)
{
    QVariantMap properties = thread.properties();

    int pos = existingPositionForEntry(properties);
    if (pos < 0) {
        qWarning() << "Could not find group for property " << properties[mGroupingProperty];
        return;
    }

    HistoryThreadGroup &group = mGroups[pos];
    group.threads.removeAll(thread);

    if (group.threads.isEmpty()) {
        removeGroup(group);
    } else if (group.displayedThread == thread) {
        updateDisplayedThread(group);
        markGroupAsChanged(group);
    }
}

void HistoryGroupedThreadsModel::markGroupAsChanged(const HistoryThreadGroup &group)
{
    if (!mChangedGroups.contains(group)) {
        mChangedGroups.append(group);
    }
}

void HistoryGroupedThreadsModel::notifyDataChanged()
{
    Q_FOREACH(const HistoryThreadGroup &group, mChangedGroups) {
        int pos = mGroups.indexOf(group);
        if (pos >= 0) {
            QModelIndex idx = index(pos);
            Q_EMIT dataChanged(idx, idx);
        } else {
            qWarning() << "Group not found!";
        }
    }
    mChangedGroups.clear();
}

QString HistoryGroupedThreadsModel::groupingProperty() const
{
    return mGroupingProperty;
}

void HistoryGroupedThreadsModel::setGroupingProperty(const QString &value)
{
    mGroupingProperty = value;
    Q_EMIT groupingPropertyChanged();

    triggerQueryUpdate();
}

int HistoryGroupedThreadsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mGroups.count();
}


bool HistoryThreadGroup::operator==(const HistoryThreadGroup &other) const
{
    return displayedThread == other.displayedThread;
}
