/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
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

#include "textchannelobserver.h"
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>

TextChannelObserver::TextChannelObserver(QObject *parent) :
    QObject(parent)
{
}

void TextChannelObserver::onTextChannelAvailable(Tp::TextChannelPtr textChannel)
{
    connect(textChannel.data(),
            SIGNAL(invalidated(Tp::DBusProxy*,const QString&, const QString&)),
            SLOT(onTextChannelInvalidated()));
    connect(textChannel.data(),
            SIGNAL(messageReceived(const Tp::ReceivedMessage&)),
            SLOT(onMessageReceived(const Tp::ReceivedMessage&)));
    connect(textChannel.data(),
            SIGNAL(messageSent(Tp::Message,Tp::MessageSendingFlags,QString)),
            SLOT(onMessageSent(Tp::Message,Tp::MessageSendingFlags,QString)));
    connect(textChannel.data(),
            SIGNAL(pendingMessageRemoved(const Tp::ReceivedMessage&)),
            SLOT(onPendingMessageRemoved(const Tp::ReceivedMessage&)));

    Q_EMIT channelAvailable(textChannel);

    // process the messages that are already pending in the channel
    Q_FOREACH(const Tp::ReceivedMessage &message, textChannel->messageQueue()) {
        Q_EMIT messageReceived(textChannel, message);
    }

    mChannels.append(textChannel);
}

void TextChannelObserver::onTextChannelInvalidated()
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    mChannels.removeAll(textChannel);
}

void TextChannelObserver::onMessageReceived(const Tp::ReceivedMessage &message)
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    if (textChannel.isNull()) {
        return;
    }

    Q_EMIT messageReceived(textChannel, message);
}

void TextChannelObserver::onMessageSent(const Tp::Message &message, Tp::MessageSendingFlags flags, const QString &sentMessageToken)
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    if (textChannel.isNull()) {
        return;
    }

    if (message.parts()[0]["skip-storage"].variant().toBool()) {
        return;
    }
 
    Q_EMIT messageSent(textChannel, message, sentMessageToken);
}

void TextChannelObserver::onPendingMessageRemoved(const Tp::ReceivedMessage &message)
{
    Tp::TextChannelPtr textChannel(qobject_cast<Tp::TextChannel*>(sender()));
    if (textChannel.isNull()) {
        return;
    }

    Q_EMIT messageRead(textChannel, message);
}
