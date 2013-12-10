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

#ifndef HISTORYTHREADMODEL_H
#define HISTORYTHREADMODEL_H

#include <QAbstractListModel>
#include "types.h"
#include "textevent.h"
#include "thread.h"

class HistoryQmlFilter;
class HistoryQmlSort;

class HistoryThreadModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(HistoryQmlFilter *filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(HistoryQmlSort *sort READ sort WRITE setSort NOTIFY sortChanged)
    Q_PROPERTY(EventType type READ type WRITE setType NOTIFY typeChanged)
    Q_ENUMS(EventType)
    Q_ENUMS(Role)
    Q_ENUMS(MatchFlag)
public:
    enum EventType {
        EventTypeText = History::EventTypeText,
        EventTypeVoice = History::EventTypeVoice
    };

    enum MatchFlag {
        MatchCaseSensitive = History::MatchCaseSensitive,
        MatchCaseInsensitive = History::MatchCaseInsensitive,
        MatchContains = History::MatchContains,
        MatchPhoneNumber = History::MatchPhoneNumber
    };

    enum Role {
        AccountIdRole = Qt::UserRole,
        ThreadIdRole,
        TypeRole,
        ParticipantsRole,
        CountRole,
        UnreadCountRole,
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
        LastEventCallDurationRole
    };

    explicit HistoryThreadModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    QHash<int, QByteArray> roleNames() const;

    HistoryQmlFilter *filter() const;
    void setFilter(HistoryQmlFilter *value);

    HistoryQmlSort *sort() const;
    void setSort(HistoryQmlSort *value);

    EventType type() const;
    void setType(EventType value);

    Q_INVOKABLE QString threadIdForParticipants(const QString &accountId,
                                                int eventType,
                                                const QStringList &participants,
                                                int matchFlags = (int)History::MatchCaseSensitive,
                                                bool create = false);
    Q_INVOKABLE bool removeThread(const QString &accountId, const QString &threadId, int eventType);

Q_SIGNALS:
    void filterChanged();
    void sortChanged();
    void typeChanged();

protected Q_SLOTS:
    void updateQuery();
    void onThreadsAdded(const History::Threads &threads);
    void onThreadsModified(const History::Threads &threads);
    void onThreadsRemoved(const History::Threads &threads);

private:
    History::ThreadViewPtr mThreadView;
    History::Threads mThreads;
    bool mCanFetchMore;
    HistoryQmlFilter *mFilter;
    HistoryQmlSort *mSort;
    EventType mType;
    QHash<int, QByteArray> mRoles;
    mutable QMap<History::TextEvent, QList<QVariant> > mAttachmentCache;
};

#endif // HISTORYTHREADMODEL_H
