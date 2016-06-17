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

#ifndef HISTORYTHREADMODEL_H
#define HISTORYTHREADMODEL_H

#include "historymodel.h"
#include "types.h"
#include "textevent.h"
#include "thread.h"

class HistoryQmlFilter;
class HistoryQmlSort;

class HistoryThreadModel : public HistoryModel
{
    Q_OBJECT
    Q_ENUMS(ThreadRole)

public:

    enum ThreadRole {
        CountRole = HistoryModel::LastRole,
        UnreadCountRole,
        ChatType,
        ChatRoomInfo,
        LastEventIdRole,
        LastEventSenderIdRole,
        LastEventTimestampRole,
        LastEventDateRole,
        LastEventNewRole,
        LastEventTextMessageRole,
        LastEventTextMessageTypeRole,
        LastEventTextMessageStatusRole,
        LastEventTextReadTimestampRole,
        LastEventTextSubjectRole,
        LastEventTextAttachmentsRole,
        LastEventCallMissedRole,
        LastEventCallDurationRole,
        LastThreadRole
    };

    explicit HistoryThreadModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    QVariant threadData(const History::Thread &thread, int role) const;

    bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    void fetchMore(const QModelIndex &parent);

    virtual QHash<int, QByteArray> roleNames() const;

    Q_INVOKABLE bool removeThreads(const QVariantList &threadsProperties);

protected Q_SLOTS:
    virtual void updateQuery();
    virtual void onThreadsAdded(const History::Threads &threads);
    virtual void onThreadsModified(const History::Threads &threads);
    virtual void onThreadsRemoved(const History::Threads &threads);

protected:
    History::Threads fetchNextPage();
    bool mCanFetchMore;
    bool mGroupThreads;

private:
    History::ThreadViewPtr mThreadView;
    History::Threads mThreads;
    QHash<int, QByteArray> mRoles;
    mutable QMap<History::TextEvent, QList<QVariant> > mAttachmentCache;
};

#endif // HISTORYTHREADMODEL_H
