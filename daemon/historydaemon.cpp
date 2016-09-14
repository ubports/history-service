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

#include "historydaemon.h"
#include "telepathyhelper_p.h"
#include "filter.h"
#include "sort.h"

#include "pluginmanager.h"
#include "plugin.h"
#include "pluginthreadview.h"
#include "plugineventview.h"
#include "textevent.h"

#include <QStandardPaths>
#include <QCryptographicHash>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/PendingVariantMap>
#include <TelepathyQt/ReferencedHandles>

Q_DECLARE_METATYPE(RolesMap)

const constexpr static int AdminRole = 2;

const QDBusArgument &operator>>(const QDBusArgument &argument, RolesMap &roles)
{
    argument.beginMap();
    while ( !argument.atEnd() ) {
        argument.beginMapEntry();
        uint key,value;
        argument >> key >> value;
        argument.endMapEntry();
        roles[key] = value;
    }

    argument.endMap();
    return argument;
}

bool foundAsMemberInThread(const Tp::ContactPtr& contact, QVariantMap thread)
{
    Q_FOREACH (QVariant participant, thread[History::FieldParticipants].toList()) {
        // found if same identifier and as member into thread info
        if (contact->id() == participant.toMap()[History::FieldIdentifier].toString() &&
                participant.toMap()[History::FieldParticipantState].toUInt() == History::ParticipantStateRegular)
        {
            return true;
        }
    }
    return false;
}

HistoryDaemon::HistoryDaemon(QObject *parent)
    : QObject(parent), mCallObserver(this), mTextObserver(this)
{
    qRegisterMetaType<RolesMap>();
    qDBusRegisterMetaType<RolesMap>();
    // get the first plugin
    if (!History::PluginManager::instance()->plugins().isEmpty()) {
        mBackend = History::PluginManager::instance()->plugins().first();
    }

    // FIXME: maybe we should only set the plugin as ready after the contact cache was generated
    connect(History::TelepathyHelper::instance(), &History::TelepathyHelper::setupReady, [&]() {
        mBackend->generateContactCache();
        mDBus.connectToBus();
    });

    connect(History::TelepathyHelper::instance(),
            SIGNAL(channelObserverCreated(ChannelObserver*)),
            SLOT(onObserverCreated()));
    History::TelepathyHelper::instance()->registerChannelObserver();

    connect(&mCallObserver,
            SIGNAL(callEnded(Tp::CallChannelPtr)),
            SLOT(onCallEnded(Tp::CallChannelPtr)));
    connect(&mTextObserver,
            SIGNAL(messageReceived(Tp::TextChannelPtr,Tp::ReceivedMessage)),
            SLOT(onMessageReceived(Tp::TextChannelPtr,Tp::ReceivedMessage)));
    connect(&mTextObserver,
            SIGNAL(messageSent(Tp::TextChannelPtr,Tp::Message,QString)),
            SLOT(onMessageSent(Tp::TextChannelPtr,Tp::Message,QString)));
    connect(&mTextObserver,
            SIGNAL(messageRead(Tp::TextChannelPtr,Tp::ReceivedMessage)),
            SLOT(onMessageRead(Tp::TextChannelPtr,Tp::ReceivedMessage)));
    connect(&mTextObserver,
            SIGNAL(channelAvailable(Tp::TextChannelPtr)),
            SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));

    // FIXME: we need to do this in a better way, but for now this should do
    mProtocolFlags["ofono"] = History::MatchPhoneNumber;
    mProtocolFlags["multimedia"] = History::MatchPhoneNumber;
}

HistoryDaemon::~HistoryDaemon()
{
}

HistoryDaemon *HistoryDaemon::instance()
{
    static HistoryDaemon *self = new HistoryDaemon();
    return self;
}

QVariantMap HistoryDaemon::propertiesFromChannel(const Tp::ChannelPtr &textChannel)
{
    QVariantMap properties;
    QVariantList participants;
    QStringList participantIds;
    RolesMap roles;

    QDBusInterface propsInterface(textChannel->busName(), textChannel->objectPath(), "org.freedesktop.DBus.Properties");
    if (propsInterface.isValid()) {
        QDBusMessage result = propsInterface.call("Get", "org.freedesktop.Telepathy.Channel.Interface.Roles", "Roles");
        roles = qdbus_cast<RolesMap>(result.arguments().at(0).value<QDBusVariant>().variant().value<QDBusArgument>());
    }

    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupContacts(false)) {
        QVariantMap contactProperties;
        contactProperties[History::FieldAlias] = contact->alias();
        contactProperties[History::FieldAccountId] = textChannel->property(History::FieldAccountId).toString();
        contactProperties[History::FieldIdentifier] = contact->id();
        contactProperties[History::FieldParticipantState] = History::ParticipantStateRegular;
        contactProperties[History::FieldParticipantRoles] = roles[contact->handle().at(0)];
        participantIds << contact->id();
        participants << contactProperties;
    }

    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupRemotePendingContacts(false)) {
        QVariantMap contactProperties;
        contactProperties[History::FieldAlias] = contact->alias();
        contactProperties[History::FieldAccountId] = textChannel->property(History::FieldAccountId).toString();
        contactProperties[History::FieldIdentifier] = contact->id();
        contactProperties[History::FieldParticipantState] = History::ParticipantStateRemotePending;
        contactProperties[History::FieldParticipantRoles] = roles[contact->handle().at(0)];
        participantIds << contact->id();
        participants << contactProperties;
    }

    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupLocalPendingContacts(false)) {
        QVariantMap contactProperties;
        contactProperties[History::FieldAlias] = contact->alias();
        contactProperties[History::FieldAccountId] = textChannel->property(History::FieldAccountId).toString();
        contactProperties[History::FieldIdentifier] = contact->id();
        contactProperties[History::FieldParticipantState] = History::ParticipantStateLocalPending;
        contactProperties[History::FieldParticipantRoles] = roles[contact->handle().at(0)];
        participantIds << contact->id();
        participants << contactProperties;
    }

    if (participants.isEmpty() && textChannel->targetHandleType() == Tp::HandleTypeContact &&
            textChannel->targetContact() == textChannel->connection()->selfContact()) {
        QVariantMap contactProperties;
        contactProperties[History::FieldAlias] = textChannel->targetContact()->alias();
        contactProperties[History::FieldAccountId] = textChannel->property(History::FieldAccountId).toString();
        contactProperties[History::FieldIdentifier] = textChannel->targetContact()->id();
        contactProperties[History::FieldParticipantState] = History::ParticipantStateRegular;
        participantIds << textChannel->targetContact()->id();
        participants << contactProperties;
    }

    // We map chatType directly from telepathy targetHandleType: None, Contact, Room
    properties[History::FieldChatType] = textChannel->targetHandleType();
    properties[History::FieldParticipants] = participants;
    properties[History::FieldParticipantIds] = participantIds;

    QVariantMap roomProperties;
    switch(textChannel->targetHandleType()) {
    case Tp::HandleTypeRoom:
        if (textChannel->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_ROOM)) {
            auto room_interface = textChannel->optionalInterface<Tp::Client::ChannelInterfaceRoomInterface>();
            QVariantMap map = getInterfaceProperties(room_interface);
            for (QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
                if (iter.value().isValid()) {
                    roomProperties[iter.key()] = iter.value();
                }
            }
        }
        if (textChannel->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_ROOM_CONFIG)) {
            auto room_config_interface = textChannel->optionalInterface<Tp::Client::ChannelInterfaceRoomConfigInterface>();
            QVariantMap map = getInterfaceProperties(room_config_interface);
            for (QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
                if (iter.value().isValid()) {
                    roomProperties[iter.key()] = iter.value();
                }
            }
        }
        if (textChannel->hasInterface(TP_QT_IFACE_CHANNEL_INTERFACE_SUBJECT)) {
            auto subject_interface = textChannel->optionalInterface<Tp::Client::ChannelInterfaceSubjectInterface>();
            QVariantMap map = getInterfaceProperties(subject_interface);
            for (QVariantMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
                if (iter.value().isValid()) {
                    roomProperties[iter.key()] = iter.value();
                }
            }
        }

        properties[History::FieldChatRoomInfo] = roomProperties;
        properties[History::FieldThreadId] = textChannel->targetId();
        break;
    case Tp::HandleTypeContact:
    case Tp::HandleTypeNone:
    default:
        break; 
    }

    return properties;
}

QVariantMap HistoryDaemon::threadForProperties(const QString &accountId,
                                               History::EventType type,
                                               const QVariantMap &properties,
                                               History::MatchFlags matchFlags,
                                               bool create)
{
    if (!mBackend) {
        return QVariantMap();
    }

    QVariantMap thread = mBackend->threadForProperties(accountId,
                                                       type,
                                                       properties,
                                                       matchFlags);
    if (thread.isEmpty() && create) {
        thread = mBackend->createThreadForProperties(accountId, type, properties);
        if (!thread.isEmpty()) {
            if (properties.contains("Requested") && properties[History::FieldChatType].toInt() == History::ChatTypeRoom) {
                QVariantMap map = thread[History::FieldChatRoomInfo].toMap();
                map["Requested"] = properties["Requested"];
                thread[History::FieldChatRoomInfo] = map;
            }
            mDBus.notifyThreadsAdded(QList<QVariantMap>() << thread);
        }
    }
    return thread;
}

QString HistoryDaemon::queryThreads(int type, const QVariantMap &sort, const QVariantMap &filter, const QVariantMap &properties)
{
    if (!mBackend) {
        return QString::null;
    }

    History::Sort theSort = History::Sort::fromProperties(sort);
    History::Filter theFilter = History::Filter::fromProperties(filter);
    History::PluginThreadView *view = mBackend->queryThreads((History::EventType)type, theSort, theFilter, properties);

    if (!view) {
        return QString::null;
    }

    // FIXME: maybe we should keep a list of views to manually remove them at some point?
    view->setParent(this);
    return view->objectPath();
}

QString HistoryDaemon::queryEvents(int type, const QVariantMap &sort, const QVariantMap &filter)
{
    if (!mBackend) {
        return QString::null;
    }

    History::Sort theSort = History::Sort::fromProperties(sort);
    History::Filter theFilter = History::Filter::fromProperties(filter);
    History::PluginEventView *view = mBackend->queryEvents((History::EventType)type, theSort, theFilter);

    if (!view) {
        return QString::null;
    }

    // FIXME: maybe we should keep a list of views to manually remove them at some point?
    view->setParent(this);
    return view->objectPath();
}

QVariantMap HistoryDaemon::getSingleThread(int type, const QString &accountId, const QString &threadId, const QVariantMap &properties)
{
    if (!mBackend) {
        return QVariantMap();
    }

    return mBackend->getSingleThread((History::EventType)type, accountId, threadId, properties);
}

QVariantMap HistoryDaemon::getSingleEvent(int type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    if (!mBackend) {
        return QVariantMap();
    }

    return mBackend->getSingleEvent((History::EventType)type, accountId, threadId, eventId);
}

bool HistoryDaemon::writeEvents(const QList<QVariantMap> &events, const QVariantMap &properties)
{
    if (!mBackend) {
        return false;
    }

    QList<QVariantMap> newEvents;
    QList<QVariantMap> modifiedEvents;
    QMap<QString, QVariantMap> threads;

    mBackend->beginBatchOperation();

    Q_FOREACH(const QVariantMap &event, events) {
        History::EventType type = (History::EventType) event[History::FieldType].toInt();
        History::EventWriteResult result;

        // get the threads for the events to notify their modifications
        QString accountId = event[History::FieldAccountId].toString();
        QString threadId = event[History::FieldThreadId].toString();
        QVariantMap savedEvent = event;

        // and finally write the event
        switch (type) {
        case History::EventTypeText:
            result = mBackend->writeTextEvent(savedEvent);
            break;
        case History::EventTypeVoice:
            result = mBackend->writeVoiceEvent(savedEvent);
            break;
        }

        // only get the thread AFTER the event is written to make sure it is up-to-date
        QVariantMap thread = getSingleThread(type, accountId, threadId, properties);
        QString hash = hashThread(thread);
        threads[hash] = thread;

        // set the participants field in the event
        savedEvent[History::FieldParticipants] = thread[History::FieldParticipants];


        // check if the event was a new one or a modification to an existing one
        switch (result) {
        case History::EventWriteCreated:
            newEvents << savedEvent;
            break;
        case History::EventWriteModified:
            modifiedEvents << savedEvent;
            break;
        case History::EventWriteError:
            mBackend->rollbackBatchOperation();
            return false;
        }
    }

    mBackend->endBatchOperation();

    // and last but not least, notify the results
    if (!newEvents.isEmpty()) {
        mDBus.notifyEventsAdded(newEvents);
    }
    if (!modifiedEvents.isEmpty()) {
        mDBus.notifyEventsModified(modifiedEvents);
    }
    if (!threads.isEmpty()) {
        mDBus.notifyThreadsModified(threads.values());
    }
    return true;
}

bool HistoryDaemon::removeEvents(const QList<QVariantMap> &events)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!mBackend) {
        return false;
    }

    mBackend->beginBatchOperation();

    Q_FOREACH(const QVariantMap &event, events) {
        History::EventType type = (History::EventType) event[History::FieldType].toInt();
        bool success = true;
        switch (type) {
        case History::EventTypeText:
            success = mBackend->removeTextEvent(event);
            break;
        case History::EventTypeVoice:
            success = mBackend->removeVoiceEvent(event);
            break;
        }

        if (!success) {
            mBackend->rollbackBatchOperation();
            return false;
        }
    }

    // now we need to get all the threads that were affected by the removal of events
    // this loop needs to be separate from the item removal loop because we rely on the
    // count property of threads to decide if they were just modified or if they need to
    // be removed.
    QMap<QString, QVariantMap> removedThreads;
    QMap<QString, QVariantMap> modifiedThreads;
    Q_FOREACH(const QVariantMap &event, events) {
         History::EventType type = (History::EventType) event[History::FieldType].toInt();
         QString accountId = event[History::FieldAccountId].toString();
         QString threadId = event[History::FieldThreadId].toString();

         QVariantMap thread = mBackend->getSingleThread(type, accountId, threadId, QVariantMap());
         if (thread.isEmpty()) {
             continue;
         }

         QString hash = hashThread(thread);
         if (thread[History::FieldCount].toInt() > 0) {
             // the thread still has items and we should notify it was modified
             modifiedThreads[hash] = thread;
         } else {
             removedThreads[hash] = thread;
         }
    }

    // finally remove the threads that are now empty
    Q_FOREACH(const QVariantMap &thread, removedThreads.values()) {
        // the thread is now empty and needs to be removed
        if (!mBackend->removeThread(thread)) {
            mBackend->rollbackBatchOperation();
            return false;
        }

    }

    mBackend->endBatchOperation();

    mDBus.notifyEventsRemoved(events);
    if (!removedThreads.isEmpty()) {
        mDBus.notifyThreadsRemoved(removedThreads.values());
    }
    if (!modifiedThreads.isEmpty()) {
        mDBus.notifyThreadsModified(modifiedThreads.values());
    }
    return true;
}

bool HistoryDaemon::removeThreads(const QList<QVariantMap> &threads)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!mBackend) {
        return false;
    }

    // In order to remove a thread all we have to do is to remove all its items
    // then it is going to be removed by removeEvents() once it detects the thread is
    // empty.
    QList<QVariantMap> events;
    QMap<QString, QVariantMap> removedEmptyThreads;
    Q_FOREACH(const QVariantMap &thread, threads) {
        QList<QVariantMap> thisEvents = mBackend->eventsForThread(thread);
        if (thisEvents.isEmpty()) {
            mBackend->beginBatchOperation();
            if (!mBackend->removeThread(thread)) {
                mBackend->rollbackBatchOperation();
                return false;
            }
            mBackend->endBatchOperation();
            QString hash = hashThread(thread);
            removedEmptyThreads[hash] = thread;
            continue;
        }
        events += thisEvents;
    }

    if (!removedEmptyThreads.isEmpty()) {
        mDBus.notifyThreadsRemoved(removedEmptyThreads.values());
    }

    if (events.size() > 0) {
        if(removeEvents(events)) {
            return true;
        }
    }

    return false;
}

void HistoryDaemon::onObserverCreated()
{
    qDebug() << __PRETTY_FUNCTION__;
    History::ChannelObserver *observer = History::TelepathyHelper::instance()->channelObserver();

    connect(observer, SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
            &mCallObserver, SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));
    connect(observer, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
            &mTextObserver, SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));
}

void HistoryDaemon::onCallEnded(const Tp::CallChannelPtr &channel)
{
    qDebug() << __PRETTY_FUNCTION__;
    QVariantMap properties = propertiesFromChannel(channel);
    QVariantList participants;
    Q_FOREACH(const Tp::ContactPtr contact, channel->remoteMembers()) {
        QVariantMap contactProperties;
        contactProperties[History::FieldAlias] = contact->alias();
        contactProperties[History::FieldIdentifier] = contact->id();
        contactProperties[History::FieldAccountId] = channel->property(History::FieldAccountId).toString();
        participants << contactProperties;
    }

    // it shouldn't happen, but in case it does, we won't crash
    if (participants.isEmpty()) {
        qWarning() << "Participants list was empty for call channel" << channel;
        return;
    }

    QString accountId = channel->property(History::FieldAccountId).toString();
    QVariantMap thread = threadForProperties(accountId,
                                             History::EventTypeVoice,
                                             properties,
                                             matchFlagsForChannel(channel),
                                             true);
    // fill the call info
    QDateTime timestamp = channel->property(History::FieldTimestamp).toDateTime();

    // FIXME: check if checking for isRequested() is enough
    bool incoming = !channel->isRequested();
    int duration;
    bool missed = incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer;

    if (!missed) {
        QDateTime activeTime = channel->property("activeTimestamp").toDateTime();
        duration = activeTime.secsTo(QDateTime::currentDateTime());
    }

    QString eventId = QString("%1:%2").arg(thread[History::FieldThreadId].toString()).arg(timestamp.toString());
    QVariantMap event;
    event[History::FieldType] = History::EventTypeVoice;
    event[History::FieldAccountId] = thread[History::FieldAccountId];
    event[History::FieldThreadId] = thread[History::FieldThreadId];
    event[History::FieldEventId] = eventId;
    event[History::FieldSenderId] = incoming ? channel->initiatorContact()->id() : "self";
    event[History::FieldTimestamp] = timestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz");
    event[History::FieldNewEvent] = missed; // only mark as a new (unseen) event if it is a missed call
    event[History::FieldMissed] = missed;
    event[History::FieldDuration] = duration;
    // FIXME: check what to do when there are more than just one remote participant
    event[History::FieldRemoteParticipant] = participants[0].toMap()[History::FieldIdentifier];
    writeEvents(QList<QVariantMap>() << event, properties);
}

void HistoryDaemon::onTextChannelAvailable(const Tp::TextChannelPtr channel)
{
    // for Rooms we need to explicitly create the thread to allow users to send messages to groups even
    // before they receive any message.
    // for other types, we can wait until messages are received
    if (channel->targetHandleType() == Tp::HandleTypeRoom) {
        QString accountId = channel->property(History::FieldAccountId).toString();
        QVariantMap properties = propertiesFromChannel(channel);

        // first try to fetch the existing thread to see if there is any.
        QVariantMap thread = threadForProperties(accountId,
                                                 History::EventTypeText,
                                                 properties,
                                                 matchFlagsForChannel(channel),
                                                 false);
        if (thread.isEmpty()) {
            // if there no existing thread, create one
            properties["Requested"] = channel->isRequested();
            thread = threadForProperties(accountId,
                                         History::EventTypeText,
                                         properties,
                                         matchFlagsForChannel(channel),
                                         true);

            // write information event including all initial invitees
            Q_FOREACH(const Tp::ContactPtr contact, channel->groupRemotePendingContacts(false)) {
                writeInformationEvent(thread, History::InformationTypeInvitationSent, contact->alias());
            }

            // update participants only if the thread is not available previously. Otherwise we'll wait for membersChanged event
            // for reflect in conversation information events for modified participants.
            updateRoomParticipants(channel, thread);
        }

        // write an entry saying you joined the group if 'joined' flag in thread is false and modify that flag.
        if (!thread[History::FieldChatRoomInfo].toMap()["Joined"].toBool()) {
            // FIXME: this is a hack. we need proper information event support
            writeInformationEvent(thread, History::InformationTypeSelfJoined);
            // update backend
            updateRoomProperties(channel, QVariantMap{{"Joined", true}});
        }

        Tp::AbstractInterface *room_interface = channel->optionalInterface<Tp::Client::ChannelInterfaceRoomInterface>();
        Tp::AbstractInterface *room_config_interface = channel->optionalInterface<Tp::Client::ChannelInterfaceRoomConfigInterface>();
        Tp::AbstractInterface *subject_interface = channel->optionalInterface<Tp::Client::ChannelInterfaceSubjectInterface>();

        QList<Tp::AbstractInterface*> interfaces;
        interfaces << room_interface << room_config_interface << subject_interface;
        for (auto interface : interfaces) {
            if (interface) {
                interface->setMonitorProperties(true);
                interface->setProperty(History::FieldAccountId, accountId);
                interface->setProperty(History::FieldThreadId, thread[History::FieldThreadId].toString());
                interface->setProperty(History::FieldType, thread[History::FieldType].toInt());
                connect(interface, SIGNAL(propertiesChanged(const QVariantMap &,const QStringList &)),
                                   SLOT(onRoomPropertiesChanged(const QVariantMap &,const QStringList &)));
                // update the stored info
                Q_EMIT interface->propertiesChanged(getInterfaceProperties(interface), QStringList());
            }
        }

        connect(channel.data(), SIGNAL(groupMembersChanged(const Tp::Contacts &, const Tp::Contacts &, const Tp::Contacts &, const Tp::Contacts &, const Tp::Channel::GroupMemberChangeDetails &)),
                SLOT(onGroupMembersChanged(const Tp::Contacts &, const Tp::Contacts &, const Tp::Contacts &, const Tp::Contacts &, const Tp::Channel::GroupMemberChangeDetails &)));
    }
}

void HistoryDaemon::onGroupMembersChanged(const Tp::Contacts &groupMembersAdded,
                                          const Tp::Contacts &groupLocalPendingMembersAdded,
                                          const Tp::Contacts &groupRemotePendingMembersAdded,
                                          const Tp::Contacts &groupMembersRemoved,
                                          const Tp::Channel::GroupMemberChangeDetails &details)
{
    Tp::TextChannelPtr channel(qobject_cast<Tp::TextChannel*>(sender()));

    QVariantMap properties;
    QVariantMap thread;

    // information events for members updates.
    bool hasMembersAdded = groupMembersAdded.size() > 0;
    //TODO: we have to take into account that removed could be in a pending list (see TelephonyService::ChatEntry for details)
    bool hasMembersRemoved = groupMembersRemoved.size() > 0;

    if (hasMembersAdded || hasMembersRemoved) {
        properties = propertiesFromChannel(channel);
        thread = threadForProperties(channel->property(History::FieldAccountId).toString(),
                                                       History::EventTypeText,
                                                       properties,
                                                       matchFlagsForChannel(channel),
                                                       false);
        if (!thread.isEmpty()) {
            if (hasMembersAdded) {
                Q_FOREACH (const Tp::ContactPtr& contact, groupMembersAdded) {
                    // if this member was not previously regular member in thread, notify about his join
                    if (!foundAsMemberInThread(contact, thread)) {
                        // FIXME: this is a hack. we need proper information event support
                        writeInformationEvent(thread, History::InformationTypeJoined, contact->alias());
                    }
                }
            }

            if (hasMembersRemoved) {
                if (channel->groupSelfContactRemoveInfo().isValid()) {
                    // evaluate if we are leaving by our own or we are kicked
                    History::InformationType type = History::InformationTypeSelfLeaving;
                    if (channel->groupSelfContactRemoveInfo().hasReason() &&
                            channel->groupSelfContactRemoveInfo().reason() == Tp::ChannelGroupChangeReason::ChannelGroupChangeReasonKicked) {
                        type = History::InformationTypeSelfKicked;
                    }
                    // FIXME: this is a hack. we need proper information event support
                    writeInformationEvent(thread, type);
                    // update backend
                    updateRoomProperties(channel, QVariantMap{{"Joined", false}});
                }
                else // don't notify any other group member removal if we are leaving the group
                {
                    Q_FOREACH (const Tp::ContactPtr& contact, groupMembersRemoved) {
                        // inform about removed members other than us
                        if (contact->id() != channel->groupSelfContact()->id()) {
                            // FIXME: this is a hack. we need proper information event support
                            writeInformationEvent(thread, History::InformationTypeLeaving, contact->alias());
                        }
                    }
                }
            }
        }
    }

    updateRoomParticipants(channel, thread);
}

void HistoryDaemon::updateRoomParticipants(const Tp::TextChannelPtr channel, const QVariantMap &thread)
{
    if (!channel) {
        return;
    }

    QVariantList participants;
    QStringList contactsAdded;
    RolesMap roles;

    QDBusInterface propsInterface(channel->busName(), channel->objectPath(), "org.freedesktop.DBus.Properties");
    if (propsInterface.isValid()) {
        QDBusMessage result = propsInterface.call("Get", "org.freedesktop.Telepathy.Channel.Interface.Roles", "Roles");
        roles = qdbus_cast<RolesMap>(result.arguments().at(0).value<QDBusVariant>().variant().value<QDBusArgument>());
    }
    Q_FOREACH(const Tp::ContactPtr contact, channel->groupRemotePendingContacts(false)) {
        QVariantMap participant;
        contactsAdded << contact->id();
        participant[History::FieldIdentifier] = contact->id();
        participant[History::FieldAlias] = contact->alias();
        participant[History::FieldParticipantState] = History::ParticipantStateRemotePending;
        participant[History::FieldParticipantRoles] = roles[contact->handle().at(0)];
        participants << QVariant::fromValue(participant);
    }
    Q_FOREACH(const Tp::ContactPtr contact, channel->groupLocalPendingContacts(false)) {
        QVariantMap participant;
        contactsAdded << contact->id();
        participant[History::FieldIdentifier] = contact->id();
        participant[History::FieldAlias] = contact->alias();
        participant[History::FieldParticipantState] = History::ParticipantStateLocalPending;
        participant[History::FieldParticipantRoles] = roles[contact->handle().at(0)];
        participants << QVariant::fromValue(participant);
    }

    Q_FOREACH(const Tp::ContactPtr contact, channel->groupContacts(false)) {
        // do not include remote and local pending members
        if (contactsAdded.contains(contact->id())) {
            continue;
        }
        QVariantMap participant;
        participant[History::FieldIdentifier] = contact->id();
        participant[History::FieldAlias] = contact->alias();
        participant[History::FieldParticipantState] = History::ParticipantStateRegular;
        participant[History::FieldParticipantRoles] = roles[contact->handle().at(0)];
        participants << QVariant::fromValue(participant);
    }

    if (!thread.isEmpty()) {
        // FIXME: this is a hack. we need proper information event support
        writeRolesInformationEvents(thread, channel, roles);
    }

    QString accountId = channel->property(History::FieldAccountId).toString();
    QString threadId = channel->targetId();
    if (mBackend->updateRoomParticipants(accountId, threadId, History::EventTypeText, participants)) {
        QVariantMap updatedThread = getSingleThread(History::EventTypeText, accountId, threadId, QVariantMap());
        mDBus.notifyThreadsModified(QList<QVariantMap>() << updatedThread);
    }

    uint selfRoles = roles[channel->groupSelfContact()->handle().at(0)];
    updateRoomProperties(channel, QVariantMap{{"SelfRoles", selfRoles}});
}

void HistoryDaemon::onRoomPropertiesChanged(const QVariantMap &properties,const QStringList &invalidated)
{
    QString accountId = sender()->property(History::FieldAccountId).toString();
    QString threadId = sender()->property(History::FieldThreadId).toString();
    History::EventType type = (History::EventType)sender()->property(History::FieldType).toInt();

    // get thread before updating to see if there are changes to insert as information events
    QVariantMap thread = getSingleThread(type, accountId, threadId, QVariantMap());
    if (!thread.empty()) {
        writeRoomChangesInformationEvents(thread, properties);
    }

    updateRoomProperties(accountId, threadId, type, properties, invalidated);
}

void HistoryDaemon::updateRoomProperties(const Tp::TextChannelPtr &channel, const QVariantMap &properties)
{
    QString accountId = channel->property(History::FieldAccountId).toString();
    QString threadId = channel->targetId();
    History::EventType type = History::EventTypeText;
    updateRoomProperties(accountId, threadId, type, properties, QStringList());
}

void HistoryDaemon::updateRoomProperties(const QString &accountId, const QString &threadId, History::EventType type, const QVariantMap &properties, const QStringList &invalidated)
{
    if (mBackend->updateRoomInfo(accountId, threadId, type, properties, invalidated)) {
        QVariantMap thread = getSingleThread(type, accountId, threadId, QVariantMap());
        mDBus.notifyThreadsModified(QList<QVariantMap>() << thread);
    }
}

void HistoryDaemon::onMessageReceived(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message)
{
    qDebug() << __PRETTY_FUNCTION__;
    QString eventId;
    Tp::MessagePart header = message.header();
    QString senderId;
    QVariantMap properties = propertiesFromChannel(textChannel);
    History::MessageStatus status = History::MessageStatusUnknown;
    if (!message.sender() || message.sender()->handle().at(0) == textChannel->connection()->selfHandle()) {
        qDebug() << __PRETTY_FUNCTION__ << message.sender();
        if (message.sender()) {
            qDebug() << __PRETTY_FUNCTION__ << "size: " << message.sender()->handle().size() << "first handle" << message.sender()->handle().at(0);
        }
        senderId = "self";
        status = History::MessageStatusDelivered;
    } else {
        senderId = message.sender()->id();
    }
    if (message.messageToken().isEmpty()) {
        eventId = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    } else {
        eventId = message.messageToken();
    }
 
    // ignore delivery reports for now.
    // FIXME: maybe we should set the readTimestamp when a delivery report is received
    if (message.isRescued()) {
        return;
    }

    if (message.isDeliveryReport() && message.deliveryDetails().hasOriginalToken()) {
        // at this point we assume the delivery report is for a message that was already
        // sent and properly saved at our database, so we can safely get it here to update
        QVariantMap textEvent = getSingleEventFromTextChannel(textChannel, message.deliveryDetails().originalToken());
        if (textEvent.isEmpty()) {
            qWarning() << "Cound not find the original event to update with delivery details.";
            return;
        }

        // FIXME: if this message is already read, don't allow reverting the status.
        // we need to check if this is the right place to do it.
        if (textEvent[History::FieldMessageStatus].toInt() == History::MessageStatusRead) {
            qWarning() << "Skipping delivery report as it is trying to revert the Read status of an existing message to the following status:" << message.deliveryDetails().status();
            return;
        }

        History::MessageStatus status;
        switch (message.deliveryDetails().status()) {
        case Tp::DeliveryStatusAccepted:
            status = History::MessageStatusAccepted;
            break;
        case Tp::DeliveryStatusDeleted:
            status = History::MessageStatusDeleted;
            break;
        case Tp::DeliveryStatusDelivered:
            status = History::MessageStatusDelivered;
            break;
        case Tp::DeliveryStatusPermanentlyFailed:
            status = History::MessageStatusPermanentlyFailed;
            break;
        case Tp::DeliveryStatusRead:
            status = History::MessageStatusRead;
            break;
        case Tp::DeliveryStatusTemporarilyFailed:
            status = History::MessageStatusTemporarilyFailed;
            break;
        case Tp::DeliveryStatusUnknown:
            status = History::MessageStatusUnknown;
            break;
        }

        textEvent[History::FieldMessageStatus] = (int) status;
        if (!writeEvents(QList<QVariantMap>() << textEvent, properties)) {
            qWarning() << "Failed to save the new message status!";
        }

        return;
    }

    QVariantMap thread = threadForProperties(textChannel->property(History::FieldAccountId).toString(),
                                                                   History::EventTypeText,
                                                                   properties,
                                                                   matchFlagsForChannel(textChannel),
                                                                   true);
    int count = 1;
    QList<QVariantMap> attachments;
    History::MessageType type = History::MessageTypeText;
    QString subject;

    if (message.hasNonTextContent()) {
        QString normalizedAccountId = QString(QCryptographicHash::hash(thread[History::FieldAccountId].toString().toLatin1(), QCryptographicHash::Md5).toHex());
        QString normalizedThreadId = QString(QCryptographicHash::hash(thread[History::FieldThreadId].toString().toLatin1(), QCryptographicHash::Md5).toHex());
        QString normalizedEventId = QString(QCryptographicHash::hash(eventId.toLatin1(), QCryptographicHash::Md5).toHex());
        QString mmsStoragePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

        type = History::MessageTypeMultiPart;
        subject = message.header()["subject"].variant().toString();

        QDir dir(mmsStoragePath);
        if (!dir.exists("history-service") && !dir.mkpath("history-service")) {
            qDebug() << "Failed to create dir";
            return;
        }
        dir.cd("history-service");

        Q_FOREACH(const Tp::MessagePart &part, message.parts()) {
            // ignore the header part
            if (part["content-type"].variant().toString().isEmpty()) {
                continue;
            }
            mmsStoragePath = dir.absoluteFilePath(QString("attachments/%1/%2/%3/").
                                                  arg(normalizedAccountId,
                                                      normalizedThreadId,
                                                      normalizedEventId));

            QFile file(mmsStoragePath+QString::number(count++));
            if (!dir.mkpath(mmsStoragePath) || !file.open(QIODevice::WriteOnly)) {
                qWarning() << "Failed to save attachment";
                continue;
            }
            file.write(part["content"].variant().toByteArray());
            file.close();

            QVariantMap attachment;
            attachment[History::FieldAccountId] = thread[History::FieldAccountId];
            attachment[History::FieldThreadId] = thread[History::FieldThreadId];
            attachment[History::FieldEventId] = eventId;
            attachment[History::FieldAttachmentId] = part["identifier"].variant();
            attachment[History::FieldContentType] = part["content-type"].variant();
            attachment[History::FieldFilePath] = file.fileName();
            attachment[History::FieldStatus] = (int) History::AttachmentDownloaded;
            attachments << attachment;
        }
    }

    QVariantMap event;
    event[History::FieldType] = History::EventTypeText;
    event[History::FieldAccountId] = thread[History::FieldAccountId];
    event[History::FieldThreadId] = thread[History::FieldThreadId];
    event[History::FieldEventId] = eventId;
    event[History::FieldSenderId] = senderId;
    event[History::FieldTimestamp] = message.received().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    event[History::FieldNewEvent] = true; // message is always unread until it reaches HistoryDaemon::onMessageRead
    event[History::FieldMessage] = message.text();
    event[History::FieldMessageType] = (int)type;
    event[History::FieldMessageStatus] = (int)status;
    event[History::FieldReadTimestamp] = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    event[History::FieldSubject] = subject;
    event[History::FieldAttachments] = QVariant::fromValue(attachments);

    writeEvents(QList<QVariantMap>() << event, properties);

    // if this messages supersedes another one, remove the original message
    if (!message.supersededToken().isEmpty()) {
        event[History::FieldEventId] = message.supersededToken();
        removeEvents(QList<QVariantMap>() << event);
    }
}

QVariantMap HistoryDaemon::getSingleEventFromTextChannel(const Tp::TextChannelPtr textChannel, const QString &messageId)
{
    QVariantMap properties = propertiesFromChannel(textChannel);

    QVariantMap thread = threadForProperties(textChannel->property(History::FieldAccountId).toString(),
                                                                     History::EventTypeText,
                                                                     properties,
                                                                     matchFlagsForChannel(textChannel),
                                                                     false);
    if (thread.isEmpty()) {
        qWarning() << "Cound not find the thread related to this eventId.";
        return QVariantMap();
    }

    QVariantMap textEvent = getSingleEvent((int)History::EventTypeText,
                                           textChannel->property(History::FieldAccountId).toString(),
                                           thread[History::FieldThreadId].toString(),
                                           messageId);

    return textEvent;

}

void HistoryDaemon::onMessageRead(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message)
{
    QVariantMap textEvent = getSingleEventFromTextChannel(textChannel, message.messageToken());
    QVariantMap properties = propertiesFromChannel(textChannel);

    if (textEvent.isEmpty()) {
        qWarning() << "Cound not find the original event to update with newEvent = false.";
        return;
    }

    textEvent[History::FieldNewEvent] = false;
    if (!writeEvents(QList<QVariantMap>() << textEvent, properties)) {
        qWarning() << "Failed to save the new message status!";
    }
}

void HistoryDaemon::onMessageSent(const Tp::TextChannelPtr textChannel, const Tp::Message &message, const QString &messageToken)
{
    qDebug() << __PRETTY_FUNCTION__;
    QVariantMap properties = propertiesFromChannel(textChannel);
    QList<QVariantMap> attachments;
    History::MessageType type = History::MessageTypeText;
    int count = 1;
    QString subject;
    QString eventId;

    if (messageToken.isEmpty()) {
        eventId = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    } else {
        eventId = messageToken;
    }
 
    QVariantMap thread = threadForProperties(textChannel->property(History::FieldAccountId).toString(),
                                              History::EventTypeText,
                                              properties,
                                              matchFlagsForChannel(textChannel),
                                              true);
    if (message.hasNonTextContent()) {
        QString normalizedAccountId = QString(QCryptographicHash::hash(thread[History::FieldAccountId].toString().toLatin1(), QCryptographicHash::Md5).toHex());
        QString normalizedThreadId = QString(QCryptographicHash::hash(thread[History::FieldThreadId].toString().toLatin1(), QCryptographicHash::Md5).toHex());
        QString normalizedEventId = QString(QCryptographicHash::hash(eventId.toLatin1(), QCryptographicHash::Md5).toHex());
        QString mmsStoragePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

        type = History::MessageTypeMultiPart;
        subject = message.header()["subject"].variant().toString();

        QDir dir(mmsStoragePath);
        if (!dir.exists("history-service") && !dir.mkpath("history-service")) {
            qDebug() << "Failed to create dir";
            return;
        }
        dir.cd("history-service");

        Q_FOREACH(const Tp::MessagePart &part, message.parts()) {
            // ignore the header part
            if (part["content-type"].variant().toString().isEmpty()) {
                continue;
            }
            mmsStoragePath = dir.absoluteFilePath(QString("attachments/%1/%2/%3/").
                                                  arg(normalizedAccountId,
                                                      normalizedThreadId,
                                                      normalizedEventId));

            QFile file(mmsStoragePath+QString::number(count++));
            if (!dir.mkpath(mmsStoragePath) || !file.open(QIODevice::WriteOnly)) {
                qWarning() << "Failed to save attachment";
                continue;
            }
            file.write(part["content"].variant().toByteArray());
            file.close();

            QVariantMap attachment;
            attachment[History::FieldAccountId] = thread[History::FieldAccountId];
            attachment[History::FieldThreadId] = thread[History::FieldThreadId];
            attachment[History::FieldEventId] = eventId;
            attachment[History::FieldAttachmentId] = part["identifier"].variant();
            attachment[History::FieldContentType] = part["content-type"].variant();
            attachment[History::FieldFilePath] = file.fileName();
            attachment[History::FieldStatus] = (int) History::AttachmentDownloaded;
            attachments << attachment;
        }
    }

    QVariantMap event;
    event[History::FieldType] = History::EventTypeText;
    event[History::FieldAccountId] = thread[History::FieldAccountId];
    event[History::FieldThreadId] = thread[History::FieldThreadId];
    event[History::FieldEventId] = eventId;
    event[History::FieldSenderId] = "self";
    event[History::FieldTimestamp] = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz"); // FIXME: check why message.sent() is empty
    event[History::FieldNewEvent] =  false; // outgoing messages are never new (unseen)
    event[History::FieldMessage] = message.text();
    event[History::FieldMessageType] = type;
    if (textChannel->deliveryReportingSupport() & Tp::DeliveryReportingSupportFlagReceiveSuccesses) {
        event[History::FieldMessageStatus] = (int)History::MessageStatusUnknown;
    } else {
        event[History::FieldMessageStatus] = (int)History::MessageStatusAccepted;
    }
    event[History::FieldReadTimestamp] = QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz");
    event[History::FieldSubject] = "";
    event[History::FieldAttachments] = QVariant::fromValue(attachments);

    writeEvents(QList<QVariantMap>() << event, properties);
}

History::MatchFlags HistoryDaemon::matchFlagsForChannel(const Tp::ChannelPtr &channel)
{
    QString protocol = channel->connection()->protocolName();
    if (mProtocolFlags.contains(protocol)) {
        return mProtocolFlags[protocol];
    }

    // default to this value
    return History::MatchCaseSensitive;
}

QString HistoryDaemon::hashThread(const QVariantMap &thread)
{
    QString hash = QString::number(thread[History::FieldType].toInt());
    hash += "#-#" + thread[History::FieldAccountId].toString();
    hash += "#-#" + thread[History::FieldThreadId].toString();
    return hash;
}

QVariantMap HistoryDaemon::getInterfaceProperties(const Tp::AbstractInterface *interface)
{
    QDBusInterface propsInterface(interface->service(), interface->path(), "org.freedesktop.DBus.Properties");
    QDBusReply<QVariantMap> reply = propsInterface.call("GetAll", interface->interface());
    if (!reply.isValid()) {
        qWarning() << "Failed to fetch channel properties for interface" << interface->interface() << reply.error().message();
    }
    return reply.value();
}

// FIXME: this is a hack. we need proper information event support.
void HistoryDaemon::writeInformationEvent(const QVariantMap &thread, History::InformationType type, const QString &subject, const QString &text)
{
    History::TextEvent historyEvent = History::TextEvent(thread[History::FieldAccountId].toString(),
                                                         thread[History::FieldThreadId].toString(),
                                                         QString(QCryptographicHash::hash(QByteArray(
                                                                 (QDateTime::currentDateTime().toString() + subject + text).toLatin1()),
                                                                 QCryptographicHash::Md5).toHex()),
                                                         "self",
                                                         QDateTime::currentDateTime(),
                                                         false,
                                                         text,
                                                         History::MessageTypeInformation,
                                                         History::MessageStatusUnknown,
                                                         QDateTime::currentDateTime(),
                                                         subject,
                                                         type);
    writeEvents(QList<QVariantMap>() << historyEvent.properties(), thread);
}

void HistoryDaemon::writeRoomChangesInformationEvents(const QVariantMap &thread, const QVariantMap &interfaceProperties)
{
    if (!thread.isEmpty()) {
        // group subject
        QString storedSubject = thread[History::FieldChatRoomInfo].toMap()["Subject"].toString();
        QString newSubject = interfaceProperties["Subject"].toString();
        if (!newSubject.isEmpty() && storedSubject != newSubject) {
            //see if we have an actor. If actor is 'me', we have changed that subject
            QString actor = thread[History::FieldChatRoomInfo].toMap()["Actor"].toString();
            if (actor == "me") {
                actor = "You";
            }

            //QString prefix = actor.isEmpty() ? "Renamed" : QString("%1 renamed").arg(actor);
            //writeInformationEvent(thread, QString("%1 the group to '%2'").arg(prefix).arg(newSubject));

            if (actor.isEmpty()) {
                writeInformationEvent(thread, History::InformationTypeTitleChanged, newSubject);
            } else {
                writeInformationEvent(thread, History::InformationTypeTitleChanged, actor, newSubject);
            }
        }
    }
}

void HistoryDaemon::writeRolesInformationEvents(const QVariantMap &thread, const Tp::ChannelPtr &channel, const RolesMap &rolesMap)
{
    // list of identifiers for current channel admins
    QStringList adminIds;

    Q_FOREACH(const Tp::ContactPtr contact, channel->groupContacts(false)) {
        // see if admin role (ChannelAdminRole == 2)
        if (rolesMap[contact->handle().at(0)] & AdminRole) {
            adminIds << contact->id();
        }
    }

    Q_FOREACH (QVariant participant, thread[History::FieldParticipants].toList()) {
        QString participantId = participant.toMap()[History::FieldIdentifier].toString();
        if (adminIds.contains(participantId)) {
            // see if already was admin or not (ChannelAdminRole == 2)
            if (! (participant.toMap()[History::FieldParticipantRoles].toUInt() & AdminRole)) {
                writeInformationEvent(thread, History::InformationTypeAdminGranted, participantId);
            }
        }
    }

    //evaluate now self roles
    if (rolesMap[channel->groupSelfContact()->handle().at(0)] & AdminRole) {
        uint selfRoles = thread[History::FieldChatRoomInfo].toMap()["SelfRoles"].toUInt();
        if (! (selfRoles & AdminRole)) {
            writeInformationEvent(thread, History::InformationTypeSelfAdminGranted);
        }
    }
}
