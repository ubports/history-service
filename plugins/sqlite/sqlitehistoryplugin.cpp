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

#include "sqlitehistoryplugin.h"
#include "intersectionfilter.h"
#include "itemfactory.h"
#include "phoneutils_p.h"
#include "sqlitedatabase.h"
#include "sqlitehistoryeventview.h"
#include "sqlitehistorythreadview.h"
#include "textevent.h"
#include "texteventattachment.h"
#include "thread.h"
#include "voiceevent.h"
#include <QDebug>
#include <QStringList>
#include <QSqlError>

SQLiteHistoryPlugin::SQLiteHistoryPlugin(QObject *parent) :
    QObject(parent)
{
}

// Reader
History::PluginThreadView *SQLiteHistoryPlugin::queryThreads(History::EventType type,
                                                         const History::SortPtr &sort,
                                                         const QString &filter)
{
    return new SQLiteHistoryThreadView(this, type, sort, filter);
}

History::EventViewPtr SQLiteHistoryPlugin::queryEvents(History::EventType type,
                                                       const History::SortPtr &sort,
                                                       const History::FilterPtr &filter)
{
    return History::EventViewPtr(new SQLiteHistoryEventView(this, type, sort, filter));
}

History::ThreadPtr SQLiteHistoryPlugin::threadForParticipants(const QString &accountId,
                                                              History::EventType type,
                                                              const QStringList &participants,
                                                              History::MatchFlags matchFlags)
{
    if (participants.isEmpty()) {
        return History::ThreadPtr(0);
    }

    QSqlQuery query(SQLiteDatabase::instance()->database());

    // select all the threads the first participant is listed in, and from that list
    // check if any of the threads has all the other participants listed
    // FIXME: find a better way to do this
    QString queryString("SELECT threadId FROM thread_participants WHERE %1 AND type=:type AND accountId=:accountId");

    // FIXME: for now we just compare differently when using MatchPhoneNumber
    if (matchFlags & History::MatchPhoneNumber) {
        queryString = queryString.arg("comparePhoneNumbers(participantId, :participantId)");
    } else {
        queryString = queryString.arg("participantId=:participantId");
    }
    query.prepare(queryString);
    query.bindValue(":participantId", participants[0]);
    query.bindValue(":type", type);
    query.bindValue(":accountId", accountId);
    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return History::ThreadPtr(0);
    }

    QStringList threadIds;
    while (query.next()) {
        threadIds << query.value(0).toString();
    }

    QString existingThread;
    // now for each threadId, check if all the other participants are listed
    Q_FOREACH(const QString &threadId, threadIds) {
        query.prepare("SELECT participantId FROM thread_participants WHERE "
                      "threadId=:threadId AND type=:type AND accountId=:accountId");
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        query.bindValue(":accountId", accountId);
        if (!query.exec()) {
            qCritical() << "Error:" << query.lastError() << query.lastQuery();
            return History::ThreadPtr(0);
        }

        QStringList threadParticipants;
        while (query.next()) {
            threadParticipants << query.value(0).toString();
        }

        // and now compare the lists
        bool found = true;
        Q_FOREACH(const QString &participant, participants) {
            if (matchFlags & History::MatchPhoneNumber) {
                // we need to iterate the list and call the phone number comparing function for
                // each participant from the given thread
                bool inList = false;
                Q_FOREACH(const QString &threadParticipant, threadParticipants) {
                    if (PhoneUtils::comparePhoneNumbers(threadParticipant, participant)) {
                        inList = true;
                        break;
                    }
                }
                if (!inList) {
                    found = false;
                    break;
                }
            } else if (!threadParticipants.contains(participant)) {
                found = false;
                break;
            }
        }

        if (found) {
            existingThread = threadId;
            break;
        }
    }

    if (!existingThread.isNull()) {
        return History::ItemFactory::instance()->createThread(accountId, existingThread, type, participants);
    }

    return History::ThreadPtr();
}

History::ThreadPtr SQLiteHistoryPlugin::getSingleThread(History::EventType type, const QString &accountId, const QString &threadId)
{
    /*History::IntersectionFilterPtr intersectionFilter(new History::IntersectionFilter());
    intersectionFilter->append(History::FilterPtr(new History::Filter("accountId", accountId)));
    intersectionFilter->append(History::FilterPtr(new History::Filter("threadId", threadId)));
    History::ThreadViewPtr view = queryThreads(type, History::SortPtr(), intersectionFilter);

    History::Threads threads= view->nextPage();
    History::ThreadPtr thread;
    if (!threads.isEmpty()) {
        thread = threads.first();
    }

    return thread;*/
    //FIXME: reimplement
    return History::ThreadPtr();
}

History::EventPtr SQLiteHistoryPlugin::getSingleEvent(History::EventType type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    History::IntersectionFilterPtr intersectionFilter(new History::IntersectionFilter());
    intersectionFilter->append(History::FilterPtr(new History::Filter("accountId", accountId)));
    intersectionFilter->append(History::FilterPtr(new History::Filter("threadId", threadId)));
    intersectionFilter->append(History::FilterPtr(new History::Filter("eventId", eventId)));

    History::EventViewPtr view = queryEvents(type, History::SortPtr(), intersectionFilter);
    History::Events events = view->nextPage();
    History::EventPtr event;
    if (!events.isEmpty()) {
        event = events.first();
    }

    return event;
}

// Writer
History::ThreadPtr SQLiteHistoryPlugin::createThreadForParticipants(const QString &accountId, History::EventType type, const QStringList &participants)
{
    // WARNING: this function does NOT test to check if the thread is already created, you should check using HistoryReader::threadForParticipants()

    // Create a new thread
    // FIXME: define what the threadId will be
    QString threadId = participants.join("%");

    QSqlQuery query(SQLiteDatabase::instance()->database());
    query.prepare("INSERT INTO threads (accountId, threadId, type, count, unreadCount)"
                  "VALUES (:accountId, :threadId, :type, :count, :unreadCount)");
    query.bindValue(":accountId", accountId);
    query.bindValue(":threadId", threadId);
    query.bindValue(":type", type);
    query.bindValue(":count", 0);
    query.bindValue(":unreadCount", 0);
    if (!query.exec()) {
        qCritical() << "Error:" << query.lastError() << query.lastQuery();
        return History::ThreadPtr(0);
    }

    // and insert the participants
    Q_FOREACH(const QString &participant, participants) {
        query.prepare("INSERT INTO thread_participants (accountId, threadId, type, participantId)"
                      "VALUES (:accountId, :threadId, :type, :participantId)");
        query.bindValue(":accountId", accountId);
        query.bindValue(":threadId", threadId);
        query.bindValue(":type", type);
        query.bindValue(":participantId", participant);
        if (!query.exec()) {
            qCritical() << "Error:" << query.lastError() << query.lastQuery();
            return History::ThreadPtr(0);
        }
    }

    // and finally create the thread
    History::ThreadPtr thread = History::ItemFactory::instance()->createThread(accountId, threadId, type, participants);
    return thread;
}

bool SQLiteHistoryPlugin::removeThread(const History::ThreadPtr &thread)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    query.prepare("DELETE FROM threads WHERE accountId=:accountId AND threadId=:threadId AND type=:type");
    query.bindValue(":accountId", thread->accountId());
    query.bindValue(":threadId", thread->threadId());
    query.bindValue(":type", thread->type());

    if (!query.exec()) {
        qCritical() << "Failed to remove the thread: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryPlugin::writeTextEvent(const History::TextEventPtr &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    // FIXME: add support for checking if an event already exists

    query.prepare("INSERT INTO text_events (accountId, threadId, eventId, senderId, timestamp, newEvent, message, messageType, messageFlags, readTimestamp) "
                  "VALUES (:accountId, :threadId, :eventId, :senderId, :timestamp, :newEvent, :message, :messageType, :messageFlags, :readTimestamp)");
    query.bindValue(":accountId", event->accountId());
    query.bindValue(":threadId", event->threadId());
    query.bindValue(":eventId", event->eventId());
    query.bindValue(":senderId", event->senderId());
    query.bindValue(":timestamp", event->timestamp());
    query.bindValue(":newEvent", event->newEvent());
    query.bindValue(":message", event->message());
    query.bindValue(":messageType", event->messageType());
    query.bindValue(":messageFlags", (int) event->messageFlags());
    query.bindValue(":readTimestamp", event->readTimestamp());

    if (!query.exec()) {
        qCritical() << "Failed to save the text event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    if (event->messageType() == History::MessageTypeMultiParty) {
        // save the attachments
        Q_FOREACH(const History::TextEventAttachmentPtr &attachment, event->attachments()) {
            query.prepare("INSERT INTO text_event_attachments VALUES (:accountId, :threadId, :eventId, :attachmentId, :contentType, :filePath, :status)");
            query.bindValue(":accountId", attachment->accountId());
            query.bindValue(":threadId", attachment->threadId());
            query.bindValue(":eventId", attachment->eventId());
            query.bindValue(":attachmentId", attachment->attachmentId());
            query.bindValue(":contentType", attachment->contentType());
            query.bindValue(":filePath", attachment->filePath());
            query.bindValue(":status", attachment->status());
            if (!query.exec()) {
                qCritical() << "Failed to save attachment to database" << query.lastError() << attachment->attachmentId() << attachment->contentType();
                return false;
            }
        }
    }

    return true;
}

bool SQLiteHistoryPlugin::removeTextEvent(const History::TextEventPtr &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    query.prepare("DELETE FROM text_events WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");
    query.bindValue(":accountId", event->accountId());
    query.bindValue(":threadId", event->threadId());
    query.bindValue(":eventId", event->eventId());

    if (!query.exec()) {
        qCritical() << "Failed to save the voice event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryPlugin::writeVoiceEvent(const History::VoiceEventPtr &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    // FIXME: add support for checking if an event already exists

    query.prepare("INSERT INTO voice_events (accountId, threadId, eventId, senderId, timestamp, newEvent, duration, missed) "
                  "VALUES (:accountId, :threadId, :eventId, :senderId, :timestamp, :newEvent, :duration, :missed)");
    query.bindValue(":accountId", event->accountId());
    query.bindValue(":threadId", event->threadId());
    query.bindValue(":eventId", event->eventId());
    query.bindValue(":senderId", event->senderId());
    query.bindValue(":timestamp", event->timestamp());
    query.bindValue(":newEvent", event->newEvent());
    query.bindValue(":duration", QTime(0,0,0,0).secsTo(event->duration()));
    query.bindValue(":missed", event->missed());

    if (!query.exec()) {
        qCritical() << "Failed to save the voice event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryPlugin::removeVoiceEvent(const History::VoiceEventPtr &event)
{
    QSqlQuery query(SQLiteDatabase::instance()->database());

    query.prepare("DELETE FROM voice_events WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId");
    query.bindValue(":accountId", event->accountId());
    query.bindValue(":threadId", event->threadId());
    query.bindValue(":eventId", event->eventId());

    if (!query.exec()) {
        qCritical() << "Failed to remove the voice event: Error:" << query.lastError() << query.lastQuery();
        return false;
    }

    return true;
}

bool SQLiteHistoryPlugin::beginBatchOperation()
{
    return SQLiteDatabase::instance()->beginTransation();
}

bool SQLiteHistoryPlugin::endBatchOperation()
{
    return SQLiteDatabase::instance()->finishTransaction();
}

bool SQLiteHistoryPlugin::rollbackBatchOperation()
{
    return SQLiteDatabase::instance()->rollbackTransaction();
}

