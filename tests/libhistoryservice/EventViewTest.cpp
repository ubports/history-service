/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include <QtCore/QObject>
#include <QtTest/QtTest>

#include "eventview.h"
#include "intersectionfilter.h"
#include "manager.h"
#include "thread.h"
#include "textevent.h"
#include "unionfilter.h"
#include "voiceevent.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::MatchFlags)

#define EVENT_COUNT 50

class EventViewTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testNextPage();
    void testFilter_data();
    void testFilter();
    void testSort();

private:
    void populate();
};

void EventViewTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::MatchFlags>();

    populate();
}

void EventViewTest::testNextPage()
{
    // create a view to return all text threads and check that the right number of items get returned
    History::EventViewPtr view = History::Manager::instance()->queryEvents(History::EventTypeText);
    QVERIFY(view->isValid());
    History::Events events = view->nextPage();
    History::Events allEvents;
    while (events.count() > 0) {
        allEvents << events;
        events = view->nextPage();
    }

    QCOMPARE(allEvents.count(), EVENT_COUNT * 2 + 1); // include the group text event
    Q_FOREACH(const History::Event &event, events) {
        QCOMPARE(event.type(), History::EventTypeText);
    }
}

void EventViewTest::testFilter_data()
{
    QTest::addColumn<QVariantMap>("filterProperties");
    QTest::addColumn<History::EventType>("eventType");
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<QVariantMap>("firstEventProperties");

    History::IntersectionFilter filter;
    filter.append(History::Filter(History::FieldAccountId, "account0"));
    filter.append(History::Filter(History::FieldThreadId, "participant0"));
    filter.append(History::Filter(History::FieldEventId, "event21"));
    History::VoiceEvent voiceEvent = History::Manager::instance()->getSingleEvent(History::EventTypeVoice,
                                                                                  "account0",
                                                                                  "participant0",
                                                                                  "event21");
    QVERIFY(!voiceEvent.isNull());
    QTest::newRow("filter by accountId, threadId and eventId") << filter.properties() << History::EventTypeVoice << 1 << voiceEvent.properties();

    filter.clear();
    QStringList participants;
    participants << "groupParticipant1" << "groupParticipant2";
    History::Thread thread = History::Manager::instance()->threadForParticipants("groupAccount",
                                                                                 History::EventTypeText,
                                                                                 participants,
                                                                                 History::MatchCaseSensitive);
    QVERIFY(!thread.isNull());
    History::TextEvent textEvent = History::Manager::instance()->getSingleEvent(History::EventTypeText,
                                                                                thread.accountId(),
                                                                                thread.threadId(),
                                                                                "groupEvent0");
    QVERIFY(!textEvent.isNull());
    filter.append(History::Filter(History::FieldAccountId, thread.accountId()));
    filter.append(History::Filter(History::FieldThreadId, thread.threadId()));
    QTest::newRow("filter for a group conversation") << filter.properties() << History::EventTypeText << 1 << textEvent.properties();
}

void EventViewTest::testFilter()
{
    QFETCH(QVariantMap, filterProperties);
    QFETCH(History::EventType, eventType);
    QFETCH(int, resultCount);
    QFETCH(QVariantMap, firstEventProperties);

    History::Filter filter = History::Filter::fromProperties(filterProperties);
    History::EventViewPtr view = History::Manager::instance()->queryEvents(eventType,
                                                                           History::Sort(History::FieldAccountId),
                                                                           filter);
    QVERIFY(view->isValid());

    History::Events events = view->nextPage();
    QCOMPARE(events.count(), resultCount);

    History::Event event = events.first();
    QCOMPARE(event.properties(), firstEventProperties);

    // make sure no more items are returned
    QVERIFY(view->nextPage().isEmpty());
}

void EventViewTest::testSort()
{
    History::Sort ascendingSort(History::FieldEventId, Qt::AscendingOrder);
    History::EventViewPtr view = History::Manager::instance()->queryEvents(History::EventTypeText, ascendingSort);
    QVERIFY(view->isValid());
    History::Events allEvents;
    History::Events events = view->nextPage();
    while (!events.isEmpty()) {
        allEvents << events;
        events = view->nextPage();
    }

    QCOMPARE(allEvents.first().eventId(), QString("event00"));
    QCOMPARE(allEvents.last().eventId(), QString("groupEvent0"));

    History::Sort descendingSort(History::FieldEventId, Qt::DescendingOrder);
    allEvents.clear();
    view = History::Manager::instance()->queryEvents(History::EventTypeVoice, descendingSort);
    QVERIFY(view->isValid());
    events = view->nextPage();
    while (!events.isEmpty()) {
        allEvents << events;
        events = view->nextPage();
    }

    QCOMPARE(allEvents.first().eventId(), QString("groupEvent0"));
    QCOMPARE(allEvents.last().eventId(), QString("event00"));
}

void EventViewTest::populate()
{
    // create two threads of each type
    for (int i = 0; i < 2; ++i) {
        History::Thread voiceThread = History::Manager::instance()->threadForParticipants(QString("account%1").arg(i),
                                                                                          History::EventTypeVoice,
                                                                                          QStringList() << QString("participant%1").arg(i),
                                                                                          History::MatchCaseSensitive,
                                                                                          true);
        QVERIFY(!voiceThread.isNull());

        History::Thread textThread = History::Manager::instance()->threadForParticipants(QString("account%1").arg(i),
                                                                                         History::EventTypeText,
                                                                                         QStringList() << QString("participant%1").arg(i),
                                                                                         History::MatchCaseSensitive,
                                                                                         true);
        QVERIFY(!textThread.isNull());


        // now create some events for the threads
        History::Events events;
        for (int j = 0; j < EVENT_COUNT; ++j) {
            History::VoiceEvent voiceEvent(voiceThread.accountId(),
                                           voiceThread.threadId(),
                                           QString("event%1").arg(j, 2, 10, QChar('0')),
                                           j % 2 ? "self" : QString("participant%1").arg(i),
                                           QDateTime::currentDateTime(),
                                           j % 2,
                                           j % 2,
                                           j % 2 ? QTime(i, j, 0) : QTime());

            History::TextEvent textEvent(textThread.accountId(),
                                         textThread.threadId(),
                                         QString("event%1").arg(j, 2, 10, QChar('0')),
                                         j % 2 ? "self" : QString("participant%1").arg(i),
                                         QDateTime::currentDateTime(),
                                         j % 2,
                                         QString("Hello %1").arg(j),
                                         History::MessageTypeText,
                                         History::MessageStatusDelivered);
            events << voiceEvent << textEvent;
        }
        QVERIFY(History::Manager::instance()->writeEvents(events));
    }

    // create a text thread with multiple participants
    QStringList participants;
    participants << "groupParticipant1" << "groupParticipant2";

    History::Thread groupTextThread = History::Manager::instance()->threadForParticipants("groupAccount",
                                                                                          History::EventTypeText,
                                                                                          participants,
                                                                                          History::MatchCaseSensitive,
                                                                                          true);
    QVERIFY(!groupTextThread.isNull());

    // and write a single event to it, just to make sure it works
    History::TextEvent groupTextEvent("groupAccount",
                                      groupTextThread.threadId(),
                                      "groupEvent0",
                                      "groupSender",
                                      QDateTime::currentDateTime(),
                                      true,
                                      "A group message",
                                      History::MessageTypeText);
    QVERIFY(History::Manager::instance()->writeEvents(History::Events() << groupTextEvent));

    // create a text thread with multiple participants
    participants.clear();
    participants << "groupParticipant1" << "groupParticipant2";

    History::Thread groupVoiceThread = History::Manager::instance()->threadForParticipants("groupAccount",
                                                                                           History::EventTypeVoice,
                                                                                           participants,
                                                                                           History::MatchCaseSensitive,
                                                                                           true);
    QVERIFY(!groupVoiceThread.isNull());

    // and write a single event to it, just to make sure it works
    History::VoiceEvent groupVoiceEvent("groupAccount",
                                        groupVoiceThread.threadId(),
                                        "groupEvent0",
                                        "groupSender",
                                        QDateTime::currentDateTime(),
                                        true,
                                        true);
    QVERIFY(History::Manager::instance()->writeEvents(History::Events() << groupVoiceEvent));
}

QTEST_MAIN(EventViewTest)
#include "EventViewTest.moc"

