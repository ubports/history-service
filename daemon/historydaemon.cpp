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

#include "historydaemon.h"
#include "telepathyhelper.h"
#include "itemfactory.h"
#include "thread.h"
#include "manager.h"
#include "textevent.h"
#include "voiceevent.h"

#include <TelepathyQt/CallChannel>

HistoryDaemon::HistoryDaemon(QObject *parent)
    : QObject(parent), mCallObserver(this), mTextObserver(this)
{
    // trigger the creation of History::Manager so that plugins are loaded
    History::Manager::instance();

    connect(TelepathyHelper::instance(),
            SIGNAL(channelObserverCreated(ChannelObserver*)),
            SLOT(onObserverCreated()));

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
}

HistoryDaemon::~HistoryDaemon()
{
}

void HistoryDaemon::onObserverCreated()
{
    qDebug() << __PRETTY_FUNCTION__;
    ChannelObserver *observer = TelepathyHelper::instance()->channelObserver();

    connect(observer, SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
            &mCallObserver, SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));
    connect(observer, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
            &mTextObserver, SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));
}

void HistoryDaemon::onCallEnded(const Tp::CallChannelPtr &channel)
{
    qDebug() << __PRETTY_FUNCTION__;
    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, channel->remoteMembers()) {
        participants << contact->id();
    }

    History::ThreadPtr thread = History::Manager::instance()->threadForParticipants(channel->property("accountId").toString(),
                                                                                    History::EventTypeVoice,
                                                                                    participants,
                                                                                    true);

    // fill the call info
    QDateTime timestamp = channel->property("timestamp").toDateTime();

    // FIXME: check if checking for isRequested() is enough
    bool incoming = !channel->isRequested();
    QTime duration(0, 0, 0);
    bool missed = incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer;

    if (!missed) {
        QDateTime activeTime = channel->property("activeTimestamp").toDateTime();
        duration = duration.addSecs(activeTime.secsTo(QDateTime::currentDateTime()));
    }

    QString eventId = QString("%1:%2").arg(thread->threadId()).arg(timestamp.toString());
    History::VoiceEventPtr event = History::ItemFactory::instance()->createVoiceEvent(thread->accountId(),
                                                                                      thread->threadId(),
                                                                                      eventId,
                                                                                      incoming ? channel->initiatorContact()->id() : "self",
                                                                                      timestamp,
                                                                                      missed, // only mark as a new (unseen) event if it is a missed call
                                                                                      missed,
                                                                                      duration);
    History::Manager::instance()->writeVoiceEvents(History::VoiceEvents() << event);
}

void HistoryDaemon::onMessageReceived(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message)
{
    qDebug() << __PRETTY_FUNCTION__;

    // ignore delivery reports for now.
    // FIXME: maybe we should set the readTimestamp when a delivery report is received
    if (message.isDeliveryReport() || message.isRescued() || message.isScrollback()) {
        return;
    }

    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupContacts(false)) {
        participants << contact->id();
    }

    History::ThreadPtr thread = History::Manager::instance()->threadForParticipants(textChannel->property("accountId").toString(),
                                                                                    History::EventTypeText,
                                                                                    participants,
                                                                                    true);
    History::TextEventPtr event = History::ItemFactory::instance()->createTextEvent(thread->accountId(),
                                                                                    thread->threadId(),
                                                                                    message.messageToken(),
                                                                                    message.sender()->id(),
                                                                                    message.received(),
                                                                                    true, // message is always unread until it reaches HistoryDaemon::onMessageRead
                                                                                    message.text(),
                                                                                    History::MessageTypeText, // FIXME: add support for MMS
                                                                                    History::MessageFlags(),
                                                                                    QDateTime());
    History::Manager::instance()->writeTextEvents(History::TextEvents() << event);
}

void HistoryDaemon::onMessageRead(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message)
{
    qDebug() << __PRETTY_FUNCTION__;
    // FIXME: implement
}

void HistoryDaemon::onMessageSent(const Tp::TextChannelPtr textChannel, const Tp::Message &message, const QString &messageToken)
{
    qDebug() << __PRETTY_FUNCTION__;
    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupContacts(false)) {
        participants << contact->id();
    }

    History::ThreadPtr thread = History::Manager::instance()->threadForParticipants(textChannel->property("accountId").toString(),
                                                                                    History::EventTypeText,
                                                                                    participants,
                                                                                    true);
    History::TextEventPtr event = History::ItemFactory::instance()->createTextEvent(thread->accountId(),
                                                                                    thread->threadId(),
                                                                                    messageToken,
                                                                                    "self",
                                                                                    QDateTime::currentDateTime(), // FIXME: check why message.sent() is empty
                                                                                    false, // outgoing messages are never new (unseen)
                                                                                    message.text(),
                                                                                    History::MessageTypeText, // FIXME: add support for MMS
                                                                                    History::MessageFlags(),
                                                                                    QDateTime());
    History::Manager::instance()->writeTextEvents(History::TextEvents() << event);
}
