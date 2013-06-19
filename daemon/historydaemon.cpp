#include "historydaemon.h"
#include "telepathyhelper.h"

#include <HistoryWriter>
#include <VoiceItem>

HistoryDaemon::HistoryDaemon(QObject *parent)
    : QObject(parent), mWriter(0)
{
    connect(TelepathyHelper::instance(),
            SIGNAL(channelObserverCreated(ChannelObserver*)),
            SLOT(onObserverCreated()));
}

HistoryDaemon::~HistoryDaemon()
{
}

void HistoryDaemon::onObserverCreated()
{
    ChannelObserver *observer = TelepathyHelper::instance()->channelObserver();

    connect(observer,
            SIGNAL(callEnded(Tp::CallChannelPtr)),
            SLOT(onCallEnded(Tp::CallChannelPtr)));
    connect(observer, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
            &mTextObserver, SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));
}

void HistoryDaemon::onCallEnded(const Tp::CallChannelPtr &channel)
{
    if (!mWriter) {
        return;
    }

    // TODO: create the VoiceItem and save it
    VoiceItem item;
    mWriter->writeVoiceItem(item);
}
