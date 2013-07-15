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

#include "telepathylogimporter.h"
#include "telepathylogreader.h"
#include "writer.h"
#include "pluginmanager.h"
#include "plugin.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"
#include <TelepathyLoggerQt/Entity>


TelepathyLogImporter::TelepathyLogImporter(QObject *parent) :
    QObject(parent), mTextEvents(0), mVoiceEvents(0)
{
    Q_FOREACH(History::PluginPtr plugin, History::PluginManager::instance()->plugins()) {
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
    connect(TelepathyLogReader::instance(),
            SIGNAL(finished()),
            SLOT(onFinished()));

    qDebug() << "Starting to import...";
    mWriter->beginBatchOperation();
}

void TelepathyLogImporter::onCallEventLoaded(const Tpl::CallEventPtr &event)
{
    if (!mWriter) {
        return;
    }

    // FIXME: add support for conf call
    bool incoming = event->receiver()->entityType() == Tpl::EntityTypeSelf;
    Tpl::EntityPtr remote = incoming ? event->sender() : event->receiver();
    History::ThreadPtr thread = mWriter->threadForParticipants(event->account()->uniqueIdentifier(),
                                                               History::EventTypeVoice,
                                                               QStringList() << remote->identifier());
    QString eventId = QString("%1:%2").arg(thread->threadId()).arg(event->timestamp().toString());
    History::VoiceEventPtr historyEvent(new History::VoiceEvent(thread->accountId(),
                                                                thread->threadId(),
                                                                eventId,
                                                                incoming ? remote->identifier() : "self",
                                                                event->timestamp(),
                                                                false,
                                                                event->endReason() == Tp::CallStateChangeReasonNoAnswer,
                                                                event->duration()));
    mWriter->writeVoiceEvent(historyEvent);
    mVoiceEvents++;
}

void TelepathyLogImporter::onMessageEventLoaded(const Tpl::TextEventPtr &event)
{
    if (!mWriter) {
        return;
    }

    // FIXME: add support for conf call
    bool incoming = event->receiver()->entityType() == Tpl::EntityTypeSelf;
    Tpl::EntityPtr remote = incoming ? event->sender() : event->receiver();
    History::ThreadPtr thread = mWriter->threadForParticipants(event->account()->uniqueIdentifier(),
                                                               History::EventTypeText,
                                                               QStringList() << remote->identifier());
    History::TextEventPtr historyEvent(new History::TextEvent(thread->accountId(),
                                                              thread->threadId(),
                                                              event->messageToken(),
                                                              incoming ? remote->identifier() : "self",
                                                              event->timestamp(),
                                                              false,
                                                              event->message(),
                                                              History::TextMessage,
                                                              History::MessageFlags(),
                                                              event->timestamp()));
    mWriter->writeTextEvent(historyEvent);
    mTextEvents++;
}

void TelepathyLogImporter::onFinished()
{
    qDebug() << "... finished";
    qDebug() << "Text events:" << mTextEvents;
    qDebug() << "Voice events:" << mVoiceEvents;
    mWriter->endBatchOperation();
    qApp->quit();
}
