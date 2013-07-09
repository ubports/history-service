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
    if (!mWriter) {
        return;
    }

    qDebug() << "OnCallEnded" << channel;
    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, channel->remoteMembers()) {
        participants << contact->id();
    }

    HistoryThreadPtr thread = mWriter->threadForParticipants(channel->property("accountId").toString(),
                                                             HistoryItem::ItemTypeVoice,
                                                             participants);

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

    QString itemId = QString("%1:%2").arg(thread->threadId()).arg(timestamp.toString());
    VoiceItem item(thread->accountId(),
                   thread->threadId(),
                   itemId,
                   incoming ? channel->initiatorContact()->id() : "self",
                   timestamp,
                   missed, // only mark as a new (unseen) item if it is a missed call
                   missed,
                   duration
                   );
    mWriter->writeVoiceItem(item);
}

void HistoryDaemon::onMessageReceived(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!mWriter) {
        return;
    }

    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupContacts(false)) {
        participants << contact->id();
    }

    HistoryThreadPtr thread = mWriter->threadForParticipants(textChannel->property("accountId").toString(),
                                                             HistoryItem::ItemTypeText,
                                                             participants);
    TextItem item(thread->accountId(),
                  thread->threadId(),
                  message.messageToken(),
                  message.sender()->id(),
                  message.received(),
                  true, // message is always unread until it reaches HistoryDaemon::onMessageRead
                  message.text(),
                  TextItem::TextMessage, // FIXME: add support for MMS
                  TextItem::MessageFlags(),
                  QDateTime());
    mWriter->writeTextItem(item);
}

void HistoryDaemon::onMessageRead(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message)
{
    if (!mWriter) {
        return;
    }

    // FIXME: implement
}

void HistoryDaemon::onMessageSent(const Tp::TextChannelPtr textChannel, const Tp::Message &message, const QString &messageToken)
{
    if (!mWriter) {
        return;
    }

    QStringList participants;
    Q_FOREACH(const Tp::ContactPtr contact, textChannel->groupContacts(false)) {
        participants << contact->id();
    }

    HistoryThreadPtr thread = mWriter->threadForParticipants(textChannel->property("accountId").toString(),
                                                             HistoryItem::ItemTypeText,
                                                             participants);
    TextItem item(thread->accountId(),
                  thread->threadId(),
                  messageToken,
                  "self",
                  message.sent(),
                  false, // outgoing messages are never new (unseen)
                  message.text(),
                  TextItem::TextMessage, // FIXME: add support for MMS
                  TextItem::MessageFlags(),
                  QDateTime());
    mWriter->writeTextItem(item);
}
