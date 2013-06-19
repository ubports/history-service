#include "historydaemon.h"
#include "telepathyhelper.h"

HistoryDaemon::HistoryDaemon(QObject *parent)
: QObject(parent)
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
}
