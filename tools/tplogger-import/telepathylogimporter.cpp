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
#include "manager.h"
#include "itemfactory.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"
#include <TelepathyLoggerQt/Entity>


TelepathyLogImporter::TelepathyLogImporter(QObject *parent) :
    QObject(parent), mTextEventCount(0), mVoiceEventCount(0), mBatchSize(200)
{
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
}

void TelepathyLogImporter::onCallEventLoaded(const Tpl::CallEventPtr &event)
{
    // FIXME: add support for conf call
    bool incoming = event->receiver()->entityType() == Tpl::EntityTypeSelf;
    Tpl::EntityPtr remote = incoming ? event->sender() : event->receiver();
    History::ThreadPtr thread = History::Manager::instance()->threadForParticipants(event->account()->uniqueIdentifier(),
                                                                                    History::EventTypeVoice,
                                                                                    QStringList() << remote->identifier(),
                                                                                    History::MatchCaseSensitive,
                                                                                    true);
    QString eventId = QString("%1:%2").arg(thread->threadId()).arg(event->timestamp().toString());
    History::VoiceEventPtr historyEvent = History::ItemFactory::instance()->createVoiceEvent(thread->accountId(),
                                                                                             thread->threadId(),
                                                                                             eventId,
                                                                                             incoming ? remote->identifier() : "self",
                                                                                             event->timestamp(),
                                                                                             false,
                                                                                             event->endReason() == Tp::CallStateChangeReasonNoAnswer,
                                                                                             event->duration());

    mVoiceEvents << historyEvent;
    if (mVoiceEvents.count() >= mBatchSize) {
        History::Manager::instance()->writeVoiceEvents(mVoiceEvents);
        mVoiceEvents.clear();
    }
    mVoiceEventCount++;
}

void TelepathyLogImporter::onMessageEventLoaded(const Tpl::TextEventPtr &event)
{
    // FIXME: add support for conf call
    bool incoming = event->receiver()->entityType() == Tpl::EntityTypeSelf;
    Tpl::EntityPtr remote = incoming ? event->sender() : event->receiver();
    History::ThreadPtr thread = History::Manager::instance()->threadForParticipants(event->account()->uniqueIdentifier(),
                                                                                    History::EventTypeText,
                                                                                    QStringList() << remote->identifier(),
                                                                                    History::MatchCaseSensitive,
                                                                                    true);
    History::TextEventPtr historyEvent = History::ItemFactory::instance()->createTextEvent(thread->accountId(),
                                                                                           thread->threadId(),
                                                                                           event->messageToken(),
                                                                                           incoming ? remote->identifier() : "self",
                                                                                           event->timestamp(),
                                                                                           false,
                                                                                           event->message(),
                                                                                           History::MessageTypeText,
                                                                                           History::MessageFlags(),
                                                                                           event->timestamp());
    mTextEvents << historyEvent;
    if (mTextEvents.count() >= mBatchSize) {
        History::Manager::instance()->writeTextEvents(mTextEvents);
        mTextEvents.clear();
    }
    mTextEventCount++;
}

void TelepathyLogImporter::onFinished()
{
    // write the remaining items
    if (!mTextEvents.isEmpty()) {
       History::Manager::instance()->writeTextEvents(mTextEvents);
       mTextEvents.clear();
    }

    if (!mVoiceEvents.isEmpty()) {
        History::Manager::instance()->writeVoiceEvents(mVoiceEvents);
        mVoiceEvents.clear();
    }

    qDebug() << "... finished";
    qDebug() << "Text events:" << mTextEventCount;
    qDebug() << "Voice events:" << mVoiceEventCount;

    qApp->quit();
}
