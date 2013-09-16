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

#ifndef HISTORYDAEMON_H
#define HISTORYDAEMON_H

#include <QObject>
#include <QSharedPointer>
#include "types.h"
#include "textchannelobserver.h"
#include "callchannelobserver.h"
#include "historyservicedbus.h"

class HistoryDaemon : public QObject
{
    Q_OBJECT
public:
    ~HistoryDaemon();

    static HistoryDaemon *instance();

    QVariantMap threadForParticipants(const QString &accountId,
                                      History::EventType type,
                                      const QStringList &participants,
                                      History::MatchFlags matchFlags = History::MatchCaseSensitive,
                                      bool create = true);
    QString queryThreads(int type, const QVariantMap &sort, const QString &filter);
    QString queryEvents(int type, const QVariantMap &sort, const QString &filter);
    QVariantMap getSingleThread(int type, const QString &accountId, const QString &threadId);
    QVariantMap getSingleEvent(int type, const QString &accountId, const QString &threadId, const QString &eventId);

    bool writeEvents(const QList<QVariantMap> &events);
    bool removeEvents(const QList<QVariantMap> &events);
    bool removeThreads(const QList<QVariantMap> &threads);

private Q_SLOTS:
    void onObserverCreated();
    void onCallEnded(const Tp::CallChannelPtr &channel);
    void onMessageReceived(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message);
    void onMessageRead(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::TextChannelPtr textChannel, const Tp::Message &message, const QString &messageToken);

protected:
    History::MatchFlags matchFlagsForChannel(const Tp::ChannelPtr &channel);

private:
    HistoryDaemon(QObject *parent = 0);

    CallChannelObserver mCallObserver;
    TextChannelObserver mTextObserver;
    QMap<QString, History::MatchFlags> mProtocolFlags;
    History::PluginPtr mBackend;
    HistoryServiceDBus mDBus;
};

#endif
