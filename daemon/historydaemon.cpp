#include "historydaemon.h"
#include "telepathyhelper.h"

#include <HistoryPlugin>
#include <HistoryThread>
#include <HistoryWriter>
#include <PluginManager>
#include <VoiceItem>

#include <TelepathyQt/CallChannel>

HistoryDaemon::HistoryDaemon(QObject *parent)
    : QObject(parent), mCallObserver(this), mTextObserver(this), mWriter(0)
{
    qDebug() << "Going to load the plugins";
    // try to find a plugin that has a writer
    Q_FOREACH(HistoryPlugin *plugin, PluginManager::instance()->plugins()) {
        qDebug() << "Trying the plugin";
        mWriter = plugin->writer();
        if (mWriter) {
            break;
        }
    }

    connect(TelepathyHelper::instance(),
            SIGNAL(channelObserverCreated(ChannelObserver*)),
            SLOT(onObserverCreated()));

    connect(&mCallObserver,
            SIGNAL(callEnded(Tp::CallChannelPtr)),
            SLOT(onCallEnded(Tp::CallChannelPtr)));
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
    qDebug() << "OnCallEnded" << channel;
    if (!mWriter) {
        return;
    }

    qDebug() << "OnCallEnded" << channel;
    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, channel->remoteMembers()) {
        participants << contact->id();
    }

    HistoryThreadPtr thread = mWriter->threadForParticipants(channel->property("accountId").toString(),
                                                             HistoryItem::VoiceItem,
                                                             participants);
    // TODO: create the VoiceItem and save it
    VoiceItem item(thread->accountId(),
                   thread->threadId(),
                   "foobaritemid",
                   channel->isRequested() ? "self" : channel->targetContact()->id(),
                   QDateTime::currentDateTime(), // FIXME: get the correct timestamp
                   false // FIXME: get the correct missed state
                   );
    mWriter->writeVoiceItem(item);
}
