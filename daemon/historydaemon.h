/*
 * Copyright (C) 2013-2017 Canonical, Ltd.
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

#include <QCoreApplication>
#include <QObject>
#include <QSharedPointer>
#include "types.h"
#include "textchannelobserver.h"
#include "callchannelobserver.h"
#include "historyservicedbus.h"
#include "plugin.h"
#include "rolesinterface.h"

typedef QMap<uint,uint> RolesMap;

class HistoryDaemon : public QObject
{
    Q_OBJECT
public:
    ~HistoryDaemon();

    static HistoryDaemon *instance();

    QVariantMap propertiesFromChannel(const Tp::ChannelPtr &textChannel);
    QVariantMap threadForProperties(const QString &accountId,
                                    History::EventType type,
                                    const QVariantMap &properties,
                                    History::MatchFlags matchFlags = History::MatchCaseSensitive,
                                    bool create = true);
    QString threadIdForProperties(const QString &accountId,
                                      History::EventType type,
                                      const QVariantMap &properties,
                                      History::MatchFlags matchFlags = History::MatchCaseSensitive,
                                      bool create = true);
    QList<QVariantMap> participantsForThreads(const QList<QVariantMap> &threadIds);
    QString queryThreads(int type, const QVariantMap &sort, const QVariantMap &filter, const QVariantMap &properties);
    QString queryEvents(int type, const QVariantMap &sort, const QVariantMap &filter);
    QVariantMap getSingleThread(int type, const QString &accountId, const QString &threadId, const QVariantMap &properties);
    QVariantMap getSingleEvent(int type, const QString &accountId, const QString &threadId, const QString &eventId);
    QVariantMap getSingleEventFromTextChannel(const Tp::TextChannelPtr textChannel, const QString &messageId);

    bool writeEvents(const QList<QVariantMap> &events, const QVariantMap &properties, bool notify = true);
    bool removeEvents(const QList<QVariantMap> &events);
    bool removeEvents(int type, const QVariantMap &filter, const QVariantMap &sort);
    int eventsCount(int type, const QVariantMap &filter);
    bool removeThreads(const QList<QVariantMap> &threads);
    void markThreadsAsRead(const QList<QVariantMap> &threads);

private Q_SLOTS:
    void onObserverCreated();
    void onCallEnded(const Tp::CallChannelPtr &channel, bool missed);
    void onMessageReceived(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::TextChannelPtr textChannel, const Tp::Message &message, const QString &messageToken);
    void onTextChannelAvailable(const Tp::TextChannelPtr channel);
    void onTextChannelInvalidated(const Tp::TextChannelPtr channel);
    void onRoomPropertiesChanged(const QVariantMap &properties,const QStringList &invalidated);
    void onGroupMembersChanged(const Tp::Contacts &groupMembersAdded, const Tp::Contacts &groupLocalPendingMembersAdded,
                               const Tp::Contacts &groupRemotePendingMembersAdded, const Tp::Contacts &groupMembersRemoved,
                               const Tp::Channel::GroupMemberChangeDetails &details);
    void onRolesChanged(const HandleRolesMap &added, const HandleRolesMap &removed);

protected:
    History::MatchFlags matchFlagsForChannel(const Tp::ChannelPtr &channel);
    void updateRoomParticipants(const Tp::TextChannelPtr channel, bool notify = true);
    void updateRoomRoles(const Tp::TextChannelPtr &channel, const RolesMap &rolesMap, bool notify = true);
    QString hashThread(const QVariantMap &thread);
    static QVariantMap getInterfaceProperties(const Tp::AbstractInterface *interface);
    void updateRoomProperties(const Tp::TextChannelPtr &channel, const QVariantMap &properties, bool notify = true);
    void updateRoomProperties(const QString &accountId, const QString &threadId, History::EventType type, const QVariantMap &properties, const QStringList &invalidated, bool notify = true);

    void writeInformationEvent(const QVariantMap &thread, History::InformationType type, const QString &subject = QString(), const QString &sender = QString("self"), const QString &text = QString(), bool notify = true);

    void writeRoomChangesInformationEvents(const QVariantMap &thread, const QVariantMap &interfaceProperties);
    void writeRolesInformationEvents(const QVariantMap &thread, const Tp::ChannelPtr &channel, const RolesMap &rolesMap);
    void writeRolesChangesInformationEvents(const QVariantMap &thread, const Tp::ChannelPtr &channel, const RolesMap &rolesMap);

    static History::MessageStatus fromTelepathyDeliveryStatus(Tp::DeliveryStatus deliveryStatus);
    static History::ChatType fromTelepathyHandleType(const Tp::HandleType &type);
private:
    HistoryDaemon(QObject *parent = 0);

    CallChannelObserver mCallObserver;
    TextChannelObserver mTextObserver;
    QMap<QString, History::MatchFlags> mProtocolFlags;
    History::PluginPtr mBackend;
    HistoryServiceDBus mDBus;
    QMap<QString, RolesMap> mRolesMap;
};

#endif
