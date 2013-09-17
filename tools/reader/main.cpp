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

#include "manager.h"
#include "eventview.h"
#include "filter.h"
#include "intersectionfilter.h"
#include "thread.h"
#include "threadview.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QCoreApplication>
#include <QDebug>

void printEvent(const History::Event &event)
{
    QString extraInfo;
    History::TextEvent textEvent;
    History::VoiceEvent voiceEvent;

    switch (event.type()) {
    case History::EventTypeText:
        textEvent = event;
        extraInfo = QString("message: %1").arg(textEvent.message());
        break;
    case History::EventTypeVoice:
        voiceEvent = event;
        extraInfo = QString("missed: %1\n      duration: %2").arg(voiceEvent.missed() ? "yes" : "no", voiceEvent.duration().toString());
        break;
    }

    qDebug() << qPrintable(QString("    * Event: accountId: %1\n      threadId: %2\n      eventId: %3\n      senderId: %4\n      timestamp: %5\n      newEvent: %6")
                .arg(event.accountId(), event.threadId(), event.eventId(), event.senderId(), event.timestamp().toString(),
                     event.newEvent() ? "yes" : "no"));
    qDebug() << qPrintable(QString("      %1").arg(extraInfo));
}

void printThread(const History::ThreadPtr &thread)
{
    QString type = "Unknown";
    switch (thread->type()) {
    case History::EventTypeText:
        type = "Text";
        break;
    case History::EventTypeVoice:
        type = "Voice";
        break;
    }

    qDebug() << qPrintable(QString("- %1 thread - accountId: %2 threadId: %3 count: %4 unreadCount: %5").arg(type,
                                                                                                thread->accountId(),
                                                                                                thread->threadId(),
                                                                                                QString::number(thread->count()),
                                                                                                QString::number(thread->unreadCount())));
    qDebug() << qPrintable(QString("    Participants: %1").arg(thread->participants().join(", ")));

    if (!thread->lastEvent().isNull()) {
        qDebug() << "    Last event:";
        printEvent(thread->lastEvent());
    }
    qDebug() << "    All events:";
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    History::Manager *manager = History::Manager::instance();

    QList<History::EventType> eventTypes;
    eventTypes << History::EventTypeText << History::EventTypeVoice;

    Q_FOREACH(History::EventType type, eventTypes) {
        History::ThreadViewPtr view = manager->queryThreads(type);
        History::Threads threads = view->nextPage();

        while (!threads.isEmpty()) {
            Q_FOREACH(const History::ThreadPtr &thread, threads) {
                printThread(thread);

                // now print the events for this thread
                History::IntersectionFilter filter;
                filter.append(History::Filter(History::FieldThreadId, thread->threadId()));
                filter.append(History::Filter(History::FieldAccountId, thread->accountId()));
                History::EventViewPtr eventView = manager->queryEvents(type, History::Sort(), filter);
                History::Events events = eventView->nextPage();
                while (!events.isEmpty()) {
                    Q_FOREACH(const History::Event &event, events) {
                        printEvent(event);
                    }
                    events = eventView->nextPage();
                }
            }
            threads = view->nextPage();
        }
    }
}
