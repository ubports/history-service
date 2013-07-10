#include "telepathylogimporter.h"
#include "telepathylogreader.h"
#include <HistoryWriter>
#include <TelepathyLoggerQt/Entity>
#include <PluginManager>
#include <HistoryPlugin>
#include <HistoryThread>

TelepathyLogImporter::TelepathyLogImporter(QObject *parent) :
    QObject(parent)
{
    Q_FOREACH(HistoryPlugin *plugin, PluginManager::instance()->plugins()) {
        qDebug() << "Trying the plugin";
        mWriter = plugin->writer();
        if (mWriter) {
            break;
        }
    }

    connect(TelepathyLogReader::instance(),
            SIGNAL(loadedCallEvent(Tpl::CallEventPtr)),
            SLOT(onCallEventLoaded(Tpl::CallEventPtr)));
    connect(TelepathyLogReader::instance(),
            SIGNAL(loadedMessageEvent(Tpl::TextEventPtr)),
            SLOT(onMessageEventLoaded(Tpl::TextEventPtr)));
}

void TelepathyLogImporter::onCallEventLoaded(const Tpl::CallEventPtr &event)
{
    if (!mWriter) {
        return;
    }

    // FIXME: add support for conf call
    bool incoming = event->receiver()->entityType() == Tpl::EntityTypeSelf;
    Tpl::EntityPtr remote = incoming ? event->sender() : event->receiver();
    HistoryThreadPtr thread = mWriter->threadForParticipants(event->account()->uniqueIdentifier(),
                                                             HistoryItem::ItemTypeVoice,
                                                             QStringList() << remote->identifier());
    QString itemId = QString("%1:%2").arg(thread->threadId()).arg(event->timestamp().toString());
    VoiceItem item(thread->accountId(),
                   thread->threadId(),
                   itemId,
                   incoming ? remote->identifier() : "self",
                   event->timestamp(),
                   false,
                   event->endReason() == Tp::CallStateChangeReasonNoAnswer,
                   event->duration());
    mWriter->writeVoiceItem(item);
}

void TelepathyLogImporter::onMessageEventLoaded(const Tpl::TextEventPtr &event)
{
    if (!mWriter) {
        return;
    }

    // FIXME: add support for conf call
    bool incoming = event->receiver()->entityType() == Tpl::EntityTypeSelf;
    Tpl::EntityPtr remote = incoming ? event->sender() : event->receiver();
    HistoryThreadPtr thread = mWriter->threadForParticipants(event->account()->uniqueIdentifier(),
                                                             HistoryItem::ItemTypeText,
                                                             QStringList() << remote->identifier());
    TextItem item(thread->accountId(),
                  thread->threadId(),
                  event->messageToken(),
                  incoming ? remote->identifier() : "self",
                  event->timestamp(),
                  false,
                  event->message(),
                  TextItem::TextMessage,
                  TextItem::MessageFlags(),
                  event->timestamp());

    mWriter->writeTextItem(item);
}
