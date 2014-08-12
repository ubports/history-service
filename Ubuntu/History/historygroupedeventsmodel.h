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
    Q_PROPERTY(QString groupingProperty
               READ groupingProperty
               WRITE setGroupingProperty
               NOTIFY groupingPropertyChanged)
    Q_ENUMS(GroupedRole)
public:
    enum GroupedRole {
        EventsRole = HistoryEventModel::LastRole,
        EventCountRole
    };

    explicit HistoryGroupedEventsModel(QObject *parent = 0);

    // reimplemented from HistoryEventModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent = QModelIndex());
    QHash<int, QByteArray> roleNames() const;
    Q_INVOKABLE QVariant get(int row) const;

    QString groupingProperty() const;
    void setGroupingProperty(const QString &property);

    bool isAscending() const;

Q_SIGNALS:
    void groupingPropertyChanged();

protected Q_SLOTS:
    void updateQuery();
    void onEventsAdded(const History::Events &events);
    void onEventsModified(const History::Events &events);
    void onEventsRemoved(const History::Events &events);

protected:
    bool compareParticipants(const QStringList &list1, const QStringList &list2);
    bool areOfSameGroup(const History::Event &event1, const History::Event &event2);
    void addEventToGroup(const History::Event &event, HistoryEventGroup &group, int row);
    bool lessThan(const History::Event &left, const History::Event &right);

private:
    QString mGroupingProperty;
    QList<HistoryEventGroup> mEventGroups;
};

#endif // HistoryGroupedEventsModel_H
