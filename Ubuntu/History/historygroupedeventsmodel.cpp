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

HistoryGroupedEventsModel::HistoryGroupedEventsModel(QObject *parent) :
    HistoryEventModel(parent)
{
    // create the view and get some objects
    updateQuery();
}

int HistoryGroupedEventsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return mEventGroups.count();
}

QVariant HistoryGroupedEventsModel::data(const QModelIndex &index, int role) const
{
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
    if (!events.count()) {
        return;
    }

    Q_FOREACH(const History::Event &event, events) {
        int pos = positionForEvent(event);

        // check if the event belongs to the group at the position
        if (pos >= 0 && pos < mEventGroups.count()) {
            HistoryEventGroup &group = mEventGroups[pos];
            if (areOfSameGroup(event, group.displayedEvent)) {
                addEventToGroup(event, group, pos);
                continue;
            }
        }

        // else, we just create a new group
        beginInsertRows(QModelIndex(), pos, pos);
        HistoryEventGroup group;
        group.displayedEvent = event;
        group.events << event;
        mEventGroups.insert(pos, group);
        endInsertRows();

    }
}

void HistoryGroupedEventsModel::onEventsModified(const History::Events &events)
{
    // FIXME: we are not yet handling events changing the property used for sorting
    // so for now the behavior is to find the item and check if it needs inserting or
    // updating in the group, which is exactly what onEventsAdded() does, so:
    onEventsAdded(events);
}

void HistoryGroupedEventsModel::onEventsRemoved(const History::Events &events)
{
    Q_FOREACH(const History::Event &event, events) {
        int pos = positionForEvent(event);
        if (pos < 0 || pos >= rowCount()) {
            continue;
        }
        HistoryEventGroup &group = mEventGroups[pos];
        if (!group.events.contains(event)) {
            continue;
        }
        removeEventFromGroup(event, group, pos);
    }
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

    Q_FOREACH(const QString &property, mGroupingProperties) {
        // first check if the property exists in the maps
        if (!props1.contains(property) || !props2.contains(property)) {
            return false;
        }

        // now check if the values are the same
        if (property == History::FieldParticipants) {
            if (!compareParticipants(props1[property].toStringList(),
                                     props2[property].toStringList())) {
                return false;
            }
        } else if (props1[property] != props2[property]) {
            return false;
        }
    }

    // if it didn't fail before, the events are indeed of the same group
    return true;
}

void HistoryGroupedEventsModel::addEventToGroup(const History::Event &event, HistoryEventGroup &group, int row)
{
    if (!group.events.contains(event)) {
        // insert the event in the correct position according to the sort criteria
        bool append = true;
        for (int i = 0; i < group.events.count(); ++i) {
            History::Event &otherEvent = group.events[i];
            if (isAscending() ? lessThan(event, otherEvent) : lessThan(otherEvent, event)) {
                group.events.insert(i, event);
                append = false;
                break;
            }
        }

        // if it is not above any item, just append it
        if (append) {
            group.events.append(event);
        }
    }

    // now check if the displayed event should be updated
    History::Event &firstEvent = group.events.first();
    if (group.displayedEvent != firstEvent) {
        group.displayedEvent = firstEvent;
        QModelIndex idx(index(row));
        Q_EMIT dataChanged(idx, idx);
    }
}

void HistoryGroupedEventsModel::removeEventFromGroup(const History::Event &event, HistoryEventGroup &group, int row)
{
    if (group.events.contains(event)) {
        group.events.removeOne(event);
    }

    if (group.events.isEmpty()) {
        beginRemoveRows(QModelIndex(), row, row);
        mEventGroups.removeAt(row);
        endRemoveRows();
        return;
    }

    if (group.displayedEvent == event) {
        // check what is the event that should be displayed
       group.displayedEvent =  group.events.first();
        Q_FOREACH(const History::Event &other, group.events) {
            if (isAscending() ? lessThan(other, group.displayedEvent) : lessThan(group.displayedEvent, other)) {
                group.displayedEvent = other;
            }
        }
    }
    QModelIndex idx = index(row);
    Q_EMIT dataChanged(idx, idx);
}

bool HistoryGroupedEventsModel::lessThan(const History::Event &left, const History::Event &right) const
{

    QVariant leftValue = left.properties()[sort()->sortField()];
    QVariant rightValue = right.properties()[sort()->sortField()];
    return leftValue < rightValue;
}

int HistoryGroupedEventsModel::positionForEvent(const History::Event &event) const
{
    // do a binary search for the item position on the list
    int lowerBound = 0;
    int upperBound = mEventGroups.count() - 1;
    if (upperBound < 0) {
        return 0;
    }

    while (true) {
        int pos = (upperBound + lowerBound) / 2;
        const History::Event &posEvent = mEventGroups[pos].displayedEvent;
        if (lowerBound == pos) {
            if (isAscending() ? lessThan(event, posEvent) : lessThan(posEvent, event)) {
                return pos;
            }
        }
        if (isAscending() ? lessThan(posEvent, event) : lessThan(event, posEvent)) {
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

QVariant HistoryGroupedEventsModel::get(int row) const
{
    if (row >= rowCount() || row < 0) {
        return QVariant();
    }

    return data(index(row), EventsRole);
}


QStringList HistoryGroupedEventsModel::groupingProperties() const
{
    return mGroupingProperties;
}

void HistoryGroupedEventsModel::setGroupingProperties(const QStringList &properties)
{
    mGroupingProperties = properties;
    Q_EMIT groupingPropertiesChanged();
    updateQuery();
}

bool HistoryGroupedEventsModel::isAscending() const
{
    return sort() && sort()->sort().sortOrder() == Qt::AscendingOrder;
}
