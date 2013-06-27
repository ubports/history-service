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
}


void CallChannelObserver::onCallStateChanged(Tp::CallState state)
{
    Tp::CallChannel *channel = qobject_cast<Tp::CallChannel*>(sender());
    if (!channel) {
        return;
    }

    switch (state) {
    case Tp::CallStateEnded:
        Q_EMIT callEnded(Tp::CallChannelPtr(channel));
        break;
    case Tp::CallStateActive:
        channel->setProperty("activeTimestamp", QDateTime::currentDateTime());
        break;
    }
}
