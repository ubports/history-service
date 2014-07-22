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

#include "historythreadgroupingproxymodel.h"
#include "historythreadmodel.h"
#include "phoneutils_p.h"
#include <QTimer>
#include <QDebug>

HistoryThreadGroupingProxyModel::HistoryThreadGroupingProxyModel(QObject *parent) :
    SortProxyModel(parent)
{
    connect(this, SIGNAL(sourceModelChanged()), SLOT(onSourceModelChanged()));
}

QVariant HistoryThreadGroupingProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QModelIndex sourceIndex = mapToSource(index);
    HistoryThreadGroup group = groupForSourceIndex(sourceIndex);

    // fill the result using the standard QSortFilterProxyModel data function
    // and overwrite it if necessary
    QVariant result = SortProxyModel::data(index, role);
    switch (role) {
        case HistoryThreadModel::CountRole: {
            int count = 0;
            Q_FOREACH(const QPersistentModelIndex &row, group.rows) {
                count += row.data(HistoryThreadModel::CountRole).toInt();
            }
            result = count;
            break;
        }
        case HistoryThreadModel::UnreadCountRole: {
            int count = 0;
            Q_FOREACH(const QPersistentModelIndex &row, group.rows) {
                count += row.data(HistoryThreadModel::UnreadCountRole).toInt();
            }
            result = count;
            break;
        }
        case ThreadsRole: {
            QVariantList threads;
            Q_FOREACH(const QPersistentModelIndex &row, group.rows) {
                threads << row.data(HistoryThreadModel::PropertiesRole).toMap();
            }
            result = threads;
            break;
        }
    }

    return result;
}

QHash<int, QByteArray> HistoryThreadGroupingProxyModel::roleNames() const
{
    return mRoles;
}

bool HistoryThreadGroupingProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    const HistoryThreadModel *model = qobject_cast<const HistoryThreadModel*>(sourceModel());

    if (!model || !sourceIndex.isValid()) {
        return false;
    }

    HistoryThreadGroup group = groupForSourceIndex(sourceIndex);
    qDebug() << "Group displayed index: " << group.displayedIndex << " group rows" << group.rows << " source index " << sourceIndex;
    return (group.displayedIndex == sourceIndex);
}

HistoryThreadGroup & HistoryThreadGroupingProxyModel::groupForEntry(const QVariant &propertyValue) const
{
    QString finalValue;
    if (mGroupingProperty == "participants") {
        QStringList participants = propertyValue.toStringList();
        HistoryThreadGroupMap::iterator it = mGroups.begin();
        for (; it != mGroups.end(); ++it) {
            if (compareParticipants(it.value().participants, participants)) {
                finalValue = it.key();
                break;
            }
        }

        if (finalValue.isEmpty()) {
            // FIXME: find a separator that is not used in any IM service id
            finalValue = participants.join("||");
            HistoryThreadGroup &group = mGroups[finalValue];
            // set participants, otherwise they will be empty and further phone comparison will fail
            group.participants = propertyValue.toStringList();
            return group;
        }
    }
    if (finalValue.isEmpty()) {
        finalValue = propertyValue.toString();
    }
    HistoryThreadGroup &group = mGroups[finalValue];
    return group;
}

void HistoryThreadGroupingProxyModel::removeGroup(const QVariant &propertyValue)
{
    if (mGroupingProperty == "participants") {
        QStringList participants = propertyValue.toStringList();
        HistoryThreadGroupMap::iterator it = mGroups.begin();
        for (; it != mGroups.end(); ++it) {
            if (compareParticipants(it.value().participants, participants)) {
                mGroups.erase(it);
                break;
            }
        }
    } else {
         mGroups.remove(propertyValue.toString());
    }
}

void HistoryThreadGroupingProxyModel::processGrouping()
{
    HistoryThreadModel *model = qobject_cast<HistoryThreadModel*>(sourceModel());
    if (!model) {
        return;
    }

    mGroups.clear();
    int count = model->rowCount();
    for (int row = 0; row < count; ++row) {
        processRowGrouping(row);
    }
}

void HistoryThreadGroupingProxyModel::processRowGrouping(int sourceRow)
{
    HistoryThreadModel *model = qobject_cast<HistoryThreadModel*>(sourceModel());
    if (!model) {
        return;
    }

    QModelIndex sourceIndex = model->index(sourceRow, 0, QModelIndex());

    QVariantMap properties = sourceIndex.data(HistoryThreadModel::PropertiesRole).toMap();
    HistoryThreadGroup &group = groupForEntry(properties[mGroupingProperty]);

    if (!group.rows.contains(sourceIndex)) {
        group.rows.append(sourceIndex);
    }

    QDateTime timestamp = sourceIndex.data(HistoryThreadModel::LastEventTimestampRole).toDateTime();
    if (timestamp > group.latestTime || !group.displayedIndex.isValid()) {
        QPersistentModelIndex oldDisplayedIndex = group.displayedIndex;
        group.latestTime = timestamp;
        group.displayedIndex = sourceIndex;

        if (oldDisplayedIndex.isValid() && oldDisplayedIndex != group.displayedIndex) {
            markIndexAsChanged(oldDisplayedIndex);
        }
        markIndexAsChanged(group.displayedIndex);
    }
}

void HistoryThreadGroupingProxyModel::removeRowFromGroup(int sourceRow)
{
    HistoryThreadModel *model = qobject_cast<HistoryThreadModel*>(sourceModel());
    if (!model) {
        return;
    }

    QModelIndex sourceIndex = model->index(sourceRow, 0, QModelIndex());
    QVariantMap properties = sourceIndex.data(HistoryThreadModel::PropertiesRole).toMap();

    HistoryThreadGroup &group = groupForEntry(properties[mGroupingProperty]);

    group.rows.removeAll(sourceIndex);
    if (group.displayedIndex == sourceIndex) {
        QDateTime latestTimestamp;
        QPersistentModelIndex latestIndex;
        Q_FOREACH(QPersistentModelIndex index, group.rows) {
            QDateTime timestamp = index.data(HistoryThreadModel::LastEventTimestampRole).toDateTime();
            if (timestamp > latestTimestamp) {
                latestTimestamp = timestamp;
                latestIndex = index;
            }
        }

        if (group.rows.isEmpty()) {
            removeGroup(properties[mGroupingProperty]);
        } else {
            group.displayedIndex = latestIndex;
            group.latestTime = latestTimestamp;
            markIndexAsChanged(group.displayedIndex);
        }
    }
    markIndexAsChanged(sourceIndex);
}

void HistoryThreadGroupingProxyModel::triggerDataChanged()
{
    if (mDataChangedTriggered) {
        return;
    }

    QTimer::singleShot(0, this, SLOT(notifyDataChanged()));
    mDataChangedTriggered = true;
}

void HistoryThreadGroupingProxyModel::markIndexAsChanged(const QModelIndex &index)
{
    if (!mChangedIndexes.contains(index)) {
        mChangedIndexes.append(index);
    }
}

void HistoryThreadGroupingProxyModel::notifyDataChanged()
{
    mRequestedDataChanged = true;
    QAbstractItemModel *model = sourceModel();
    Q_FOREACH(const QPersistentModelIndex &index, mChangedIndexes) {
        Q_EMIT model->dataChanged(index, index);
    }
    mChangedIndexes.clear();
    mRequestedDataChanged = false;
    mDataChangedTriggered = false;
}

void HistoryThreadGroupingProxyModel::onRowsInserted(const QModelIndex &parent, int start, int end)
{
    // we don't support tree models yet
    if (parent.isValid()) {
        return;
    }


    // update the group for the added indexes
    for (int row = start; row <= end; ++row) {
        processRowGrouping(row);
    }

    triggerDataChanged();
}

void HistoryThreadGroupingProxyModel::onRowsRemoved(const QModelIndex &parent, int start, int end)
{
    // we don't support tree models yet
    if (parent.isValid()) {
        return;
    }

    for (int row = start; row <= end; ++row) {
        removeRowFromGroup(row);
    }

    triggerDataChanged();
}

void HistoryThreadGroupingProxyModel::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // we don't support tree models yet
    if (topLeft.parent().isValid() || bottomRight.parent().isValid()) {
        return;
    }

    int start = topLeft.row();
    int end = bottomRight.row();

    for (int row = start; row <= end; ++row) {
        processRowGrouping(row); 
    }
    triggerDataChanged();
}

void HistoryThreadGroupingProxyModel::onSourceModelChanged()
{
    QAbstractItemModel *model = sourceModel();
    if (model) {
        setSourceModel(model);

        connect(model,
                SIGNAL(rowsInserted(QModelIndex,int,int)),
                SLOT(onRowsInserted(QModelIndex,int,int)));
        connect(model,
                SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                SLOT(onRowsRemoved(QModelIndex,int,int)));
        connect(model,
                SIGNAL(modelReset()),
                SLOT(processGrouping()));
        connect(model,
                SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                SLOT(onDataChanged(QModelIndex,QModelIndex)));
        Q_EMIT threadModelChanged();
    }

    mRoles = SortProxyModel::roleNames();
    mRoles[ThreadsRole] = "threads";

    processGrouping();
    triggerDataChanged();
}

bool HistoryThreadGroupingProxyModel::compareParticipants(const QStringList &list1, const QStringList &list2) const
{
    // FIXME: add support for match flags
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


HistoryThreadGroup HistoryThreadGroupingProxyModel::groupForSourceIndex(const QModelIndex &sourceIndex) const
{
    QVariantMap properties = sourceIndex.data(HistoryThreadModel::PropertiesRole).toMap();
    return groupForEntry(properties[mGroupingProperty]);
}


QString HistoryThreadGroupingProxyModel::groupingProperty() const
{
    return mGroupingProperty;
}

void HistoryThreadGroupingProxyModel::setGroupingProperty(const QString &value)
{
    mGroupingProperty = value;
    Q_EMIT groupingPropertyChanged();
    processGrouping();
    triggerDataChanged();
}
