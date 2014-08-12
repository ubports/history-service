/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include "historygroupedeventsmodel.h"
#include "phoneutils_p.h"
#include "sort.h"
#include "historyqmlsort.h"
#include <QDebug>

HistoryGroupedEventsModel::HistoryGroupedEventsModel(QObject *parent) :
    HistoryEventModel(parent)
{
    qDebug() << __PRETTY_FUNCTION__;
    // create the view and get some objects
    updateQuery();
}

int HistoryGroupedEventsModel::rowCount(const QModelIndex &parent) const
{
    qDebug() << __PRETTY_FUNCTION__;
    if (parent.isValid()) {
        return 0;
    }

    return mEventGroups.count();
}

QVariant HistoryGroupedEventsModel::data(const QModelIndex &index, int role) const
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!index.isValid() || index.row() < 0 || index.row() >= mEventGroups.count()) {
        return QVariant();
    }

    HistoryEventGroup group = mEventGroups[index.row()];
    QVariant result;
    QVariantList events;

    switch (role) {
    case EventsRole:
        Q_FOREACH(const History::Event &event, group.events) {
            events << event.properties();
        }
        result = events;
        break;
    case EventCountRole:
        result = group.events.count();
        break;
    default:
        result = eventData(group.displayedEvent, role);
        break;
    }

    return result;
}

void HistoryGroupedEventsModel::fetchMore(const QModelIndex &parent)
{
    if (!canFetchMore(parent)) {
        return;
    }

    History::Events events = fetchNextPage();

    // History already deliver us the events in the right order
    // but we might have added new entries in the added, removed, modified events.
    // still, it is less expensive to do a sequential search starting from the bottom
    // than to do a binary search for each event, as it is very likely that the entries
    // belong to the bottom part of the model.
    Q_FOREACH(const History::Event event, events) {
        bool found = false;
        int pos = mEventGroups.count() -1;
        for (; pos >= 0; pos--) {
            HistoryEventGroup &group = mEventGroups[pos];
            if (areOfSameGroup(event, group.displayedEvent)) {
                qDebug() << "Found group for participants" << event.participants() << "at pos" << pos;
                found = true;
                addEventToGroup(event, group, pos);
                break;
            } else if (isAscending() ? lessThan(group.displayedEvent, event) : lessThan(event, group.displayedEvent)) {
                break;
            }
        }

        if (!found) {
            // the item goes into a new group right after the position found above
            pos++;
            HistoryEventGroup group;
            group.displayedEvent = event;
            group.events << event;
            beginInsertRows(QModelIndex(), pos, pos);
            mEventGroups.insert(pos, group);
            endInsertRows();
        }
    }
}

QHash<int, QByteArray> HistoryGroupedEventsModel::roleNames() const
{
    QHash<int, QByteArray> roles = HistoryEventModel::roleNames();
    roles[EventsRole] = "events";
    roles[EventCountRole] = "eventCount";
    return roles;
}

void HistoryGroupedEventsModel::updateQuery()
{
    // remove all event groups from the model
    if (!mEventGroups.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, mEventGroups.count() - 1);
        mEventGroups.clear();
        endRemoveRows();
    }

    // and ask HistoryEventModel to update the query and fetch items again
    HistoryEventModel::updateQuery();
}

void HistoryGroupedEventsModel::onEventsAdded(const History::Events &events)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!events.count()) {
        return;
    }

    //FIXME: reimplement
    /*beginInsertRows(QModelIndex(), mEvents.count(), mEvents.count() + filteredEvents.count() - 1);
    mEvents << filteredEvents;
    endInsertRows();*/
}

void HistoryGroupedEventsModel::onEventsModified(const History::Events &events)
{
    // FIXME: reimplement
    qDebug() << __PRETTY_FUNCTION__;
}

void HistoryGroupedEventsModel::onEventsRemoved(const History::Events &events)
{
    qDebug() << __PRETTY_FUNCTION__;
    // FIXME: reimplement
    /*
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
    */
}

bool HistoryGroupedEventsModel::compareParticipants(const QStringList &list1, const QStringList &list2)
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

bool HistoryGroupedEventsModel::areOfSameGroup(const History::Event &event1, const History::Event &event2)
{
    QVariantMap props1 = event1.properties();
    QVariantMap props2 = event2.properties();
    if (!props1.contains(mGroupingProperty) || !props2.contains(mGroupingProperty)) {
        return false;
    }

    if (mGroupingProperty == History::FieldParticipants) {
        return compareParticipants(props1[mGroupingProperty].toStringList(),
                                   props2[mGroupingProperty].toStringList());
    }

    return props1[mGroupingProperty] == props2[mGroupingProperty];
}

void HistoryGroupedEventsModel::addEventToGroup(const History::Event &event, HistoryEventGroup &group, int row)
{
    if (!group.events.contains(event)) {
        group.events << event;
    }

    // now check if the displayed event should be updated
    if (isAscending() ? lessThan(event, group.displayedEvent) : lessThan(group.displayedEvent, event)) {
        group.displayedEvent = event;
        QModelIndex idx(index(row));
        Q_EMIT dataChanged(idx, idx);
    }
}

bool HistoryGroupedEventsModel::lessThan(const History::Event &left, const History::Event &right)
{

    QVariant leftValue = left.properties()[sort()->sortField()];
    QVariant rightValue = right.properties()[sort()->sortField()];
    return leftValue < rightValue;
}

QVariant HistoryGroupedEventsModel::get(int row) const
{
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }

    return data(index(row), EventsRole);
}


QString HistoryGroupedEventsModel::groupingProperty() const
{
    return mGroupingProperty;
}

void HistoryGroupedEventsModel::setGroupingProperty(const QString &property)
{
    mGroupingProperty = property;
    Q_EMIT groupingPropertyChanged();
    updateQuery();
}

bool HistoryGroupedEventsModel::isAscending() const
{
    return sort() && sort()->sort().sortOrder() == Qt::AscendingOrder;
}
