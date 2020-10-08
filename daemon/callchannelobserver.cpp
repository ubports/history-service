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

#include "callchannelobserver.h"

CallChannelObserver::CallChannelObserver(QObject *parent) :
    QObject(parent)
{
}

void CallChannelObserver::onCallChannelAvailable(Tp::CallChannelPtr callChannel)
{
    // save the timestamp as a property in the call channel
    callChannel->setProperty("timestamp", QDateTime::currentDateTime());
    if (callChannel->callState() == Tp::CallStateActive) {
        callChannel->setProperty("activeTimestamp", QDateTime::currentDateTime());
    }


    connect(callChannel.data(),
                SIGNAL(callStateChanged(Tp::CallState)),
                SLOT(onCallStateChanged(Tp::CallState)));

    mChannels.append(callChannel);
    mCallStates[callChannel.data()] = callChannel->callState();
}


void CallChannelObserver::onCallStateChanged(Tp::CallState state)
{
    Tp::CallChannel *channel = qobject_cast<Tp::CallChannel*>(sender());
    if (!channel) {
        return;
    }

    switch (state) {
    case Tp::CallStateEnded: {
        bool incoming = !channel->isRequested();
        bool missed = incoming && channel->callStateReason().reason == Tp::CallStateChangeReasonNoAnswer;

        // If the call state is not missed at this point, we need to check from which state we transitioned to ended,
        // if from Initialised, it means it was indeed missed
        if (incoming && !missed) {
            missed = mCallStates[channel] == Tp::CallStateInitialised;
        }
        mCallStates.remove(channel);
        mChannels.removeOne(Tp::CallChannelPtr(channel));
        Q_EMIT callEnded(Tp::CallChannelPtr(channel), missed);
        break;
    }
    case Tp::CallStateActive:
        channel->setProperty("activeTimestamp", QDateTime::currentDateTime());
        // fall through
    default:
        mCallStates[channel] = state;
        break;
    }
}
