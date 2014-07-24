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

#ifndef HISTORYTHREADGROUPINGPROXYMODEL_H
#define HISTORYTHREADGROUPINGPROXYMODEL_H

#include "sortproxymodel.h"
#include <QDateTime>

class HistoryThreadModel;

class HistoryThreadGroup {
public:
    QStringList participants;
    QDateTime latestTime;
    QPersistentModelIndex displayedIndex;
    QList<QPersistentModelIndex> rows;
};

typedef QMap<QString, HistoryThreadGroup> HistoryThreadGroupMap;

class HistoryThreadGroupingProxyModel : public SortProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString groupingProperty
               READ groupingProperty
               WRITE setGroupingProperty
               NOTIFY groupingPropertyChanged)
    Q_ENUMS(CustomRoles)

public:
    enum CustomRoles {
        ThreadsRole = (Qt::UserRole + 100),
    };

    explicit HistoryThreadGroupingProxyModel(QObject *parent = 0);

    QString groupingProperty() const;
    void setGroupingProperty(const QString &value);

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;

Q_SIGNALS:
    void threadModelChanged();
    void groupingPropertyChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    HistoryThreadGroup groupForSourceIndex(const QModelIndex &sourceIndex) const;
    HistoryThreadGroup &groupForEntry(const QVariant &propertyValue) const;
    void removeGroup(const QVariant &propertyValue);

private Q_SLOTS:
    void processGrouping();
    void processRowGrouping(int sourceRow);
    void removeRowFromGroup(int sourceRow);
    void triggerDataChanged();
    void markIndexAsChanged(const QModelIndex &index);
    void notifyDataChanged();

    void onRowsInserted(const QModelIndex &parent, int start, int end);
    void onRowsRemoved(const QModelIndex &parent, int start, int end);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onSourceModelChanged();

protected:
    bool compareParticipants(const QStringList &list1, const QStringList &list2) const;

private:
    bool mDataChangedTriggered;
    QString mGroupingProperty;

    mutable HistoryThreadGroupMap mGroups;
    QList<QPersistentModelIndex> mChangedIndexes;
    QHash<int, QByteArray> mRoles;
};

#endif // HISTORYTHREADGROUPINGPROXYMODEL_H
