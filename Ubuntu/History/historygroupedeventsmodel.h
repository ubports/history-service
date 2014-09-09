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

#ifndef HISTORYGROUPEDEVENTSMODEL_H
#define HISTORYGROUPEDEVENTSMODEL_H

#include "historyeventmodel.h"

typedef struct {
    History::Events events;
    History::Event displayedEvent;
} HistoryEventGroup;

class HistoryGroupedEventsModel : public HistoryEventModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList groupingProperties
               READ groupingProperties
               WRITE setGroupingProperties
               NOTIFY groupingPropertiesChanged)
    Q_ENUMS(GroupedRole)
public:
    enum GroupedRole {
        EventsRole = HistoryEventModel::LastEventRole,
        EventCountRole
    };

    explicit HistoryGroupedEventsModel(QObject *parent = 0);

    // reimplemented from HistoryEventModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent = QModelIndex());
    QHash<int, QByteArray> roleNames() const;
    Q_INVOKABLE QVariant get(int row) const;

    QStringList groupingProperties() const;
    void setGroupingProperties(const QStringList &properties);

Q_SIGNALS:
    void groupingPropertiesChanged();

protected Q_SLOTS:
    void updateQuery();
    void onEventsAdded(const History::Events &events);
    void onEventsModified(const History::Events &events);
    void onEventsRemoved(const History::Events &events);

protected:
    bool areOfSameGroup(const History::Event &event1, const History::Event &event2);
    void addEventToGroup(const History::Event &event, HistoryEventGroup &group, int row);
    void removeEventFromGroup(const History::Event &event, HistoryEventGroup &group, int row);

private:
    QStringList mGroupingProperties;
    QList<HistoryEventGroup> mEventGroups;
};

#endif // HISTORYGROUPEDEVENTSMODEL_H
