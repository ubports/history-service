#include "historydaemon.h"
#include "telepathyhelper.h"

HistoryDaemon::HistoryDaemon(QObject *parent)
: QObject(parent)
{
    connect(TelepathyHelper::instance(),
            SIGNAL(channelObserverCreated()),
            SLOT(onObserverCreated()));
}

HistoryDaemon::~HistoryDaemon()
{
}

void HistoryDaemon::onObserverCreated()
{
}

void HistoryDaemon::onCallEnded(const Tp::CallChannelPtr &channel)
{
}
