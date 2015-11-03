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

#ifndef HISTORYGROUPEDTHREADSMODEL_H
#define HISTORYGROUPEDTHREADSMODEL_H

#include "historythreadmodel.h"
#include <QDateTime>

class HistoryThreadGroup {
public:
    History::Thread displayedThread;
    History::Threads threads;

    bool operator==(const HistoryThreadGroup &other) const;
};

typedef QList<HistoryThreadGroup> HistoryThreadGroupList;

class HistoryGroupedThreadsModel : public HistoryThreadModel
{
    Q_OBJECT
    Q_PROPERTY(QString groupingProperty
               READ groupingProperty
               WRITE setGroupingProperty
               NOTIFY groupingPropertyChanged)
    Q_ENUMS(CustomRoles)

public:
    enum CustomRoles {
        ThreadsRole = LastThreadRole
    };

    explicit HistoryGroupedThreadsModel(QObject *parent = 0);

    QString groupingProperty() const;
    void setGroupingProperty(const QString &value);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE void fetchMore(const QModelIndex &parent = QModelIndex());
    virtual QHash<int, QByteArray> roleNames() const;
    Q_INVOKABLE QVariant get(int row) const;

Q_SIGNALS:
    void groupingPropertyChanged();

protected:
    int existingPositionForEntry(const History::Thread &thread) const;
    void removeGroup(const HistoryThreadGroup &group);
    void updateDisplayedThread(HistoryThreadGroup &group);

protected Q_SLOTS:
    virtual void updateQuery();
    virtual void onThreadsAdded(const History::Threads &threads);
    virtual void onThreadsModified(const History::Threads &threads);
    virtual void onThreadsRemoved(const History::Threads &threads);

private Q_SLOTS:
    void processThreadGrouping(const History::Thread &thread);
    void removeThreadFromGroup(const History::Thread &thread);
    void markGroupAsChanged(const HistoryThreadGroup &group);
    void notifyDataChanged();

private:
    QString mGroupingProperty;

    HistoryThreadGroupList mGroups;
    QList<HistoryThreadGroup> mChangedGroups;
    QHash<int, QByteArray> mRoles;
};

#endif // HISTORYGROUPEDTHREADSMODEL_H
