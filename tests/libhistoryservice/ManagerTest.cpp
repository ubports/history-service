/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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
#include "manager.h"
#include "thread.h"
#include "threadview.h"
#include "textevent.h"
#include "voiceevent.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::MatchFlags)
Q_DECLARE_METATYPE(History::Threads)
Q_DECLARE_METATYPE(History::Events)

class ManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testThreadForParticipants_data();
    void testThreadForParticipants();
    void testQueryEvents();
    void testQueryThreads();
    void testGetSingleThread();
    void testWriteEvents();
    void testRemoveEvents();
    void testRemoveEventsByFilter();
    void testGetSingleEvent();
    void testRemoveThreads();
    void cleanup();
    void cleanupTestCase();

private:
    History::Manager *mManager;
};

void ManagerTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::MatchFlags>();
    qRegisterMetaType<History::Threads>();
    qRegisterMetaType<History::Events>();
    mManager = History::Manager::instance();
}

void ManagerTest::testThreadForParticipants_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<History::EventType>("type");
    QTest::addColumn<QStringList>("participants");
    QTest::addColumn<History::MatchFlags>("matchFlags");
    QTest::addColumn<QStringList>("participantsToMatch");

    QTest::newRow("text thread with one participant") << "oneAccountId"
                                                      << History::EventTypeText
                                                      << (QStringList() << "oneParticipant")
                                                      << History::MatchFlags(History::MatchCaseSensitive)
                                                      << (QStringList() << "oneParticipant");
    QTest::newRow("voice thread using phone match") << "anotherAccountId"
                                                    << History::EventTypeVoice
                                                    << (QStringList() << "+554198765432")
                                                    << History::MatchFlags(History::MatchPhoneNumber)
                                                    << (QStringList() << "98765432");
}

void ManagerTest::testThreadForParticipants()
{
    QFETCH(QString, accountId);
    QFETCH(History::EventType, type);
    QFETCH(QStringList, participants);
    QFETCH(History::MatchFlags, matchFlags);
    QFETCH(QStringList, participantsToMatch);

    QSignalSpy spy(mManager, SIGNAL(threadsAdded(History::Threads)));
    History::Thread thread = mManager->threadForParticipants(accountId, type, participants, matchFlags, true);
    QVERIFY(!thread.isNull());
    QTRY_COMPARE(spy.count(), 1);

    QCOMPARE(thread.accountId(), accountId);
    QCOMPARE(thread.type(), type);

    // now try to get the thread again to see if it is returned correctly
    History::Thread sameThread = mManager->threadForParticipants(accountId, type, participantsToMatch, matchFlags, false);
    QVERIFY(!sameThread.isNull());
    QCOMPARE(sameThread, thread);
}

void ManagerTest::testQueryEvents()
{
    // just make sure the view returned is not null
    // the contents of the view will be tested in its own tests
    History::EventViewPtr eventView = mManager->queryEvents(History::EventTypeText);
    QVERIFY(!eventView.isNull());
    QVERIFY(eventView->isValid());
}

void ManagerTest::testQueryThreads()
{
    // just make sure the view returned is not null
    // the contents of the view will be tested in its own tests
    History::ThreadViewPtr threadView = mManager->queryThreads(History::EventTypeVoice);
    QVERIFY(!threadView.isNull());
    QVERIFY(threadView->isValid());
}

void ManagerTest::testGetSingleThread()
{
    History::Thread thread = mManager->threadForParticipants("theAccountId",
                                                             History::EventTypeText,
                                                             QStringList() << "theParticipant",
                                                             History::MatchCaseSensitive, true);
    QVERIFY(!thread.isNull());

    // try getting the same thread
    History::Thread sameThread = mManager->getSingleThread(thread.type(), thread.accountId(), thread.threadId());
    QVERIFY(sameThread == thread);
}

void ManagerTest::testWriteEvents()
{
    QString textParticipant("textParticipant");
    QString voiceParticipant("voiceParticipant");
    // create two threads, one for voice and one for text
    History::Thread textThread = mManager->threadForParticipants("textAccountId",
                                                                 History::EventTypeText,
                                                                 QStringList()<< textParticipant,
                                                                 History::MatchCaseSensitive, true);
    History::Thread voiceThread = mManager->threadForParticipants("voiceAccountId",
                                                                  History::EventTypeVoice,
                                                                  QStringList()<< voiceParticipant,
                                                                  History::MatchCaseSensitive, true);
    // insert some text and voice events
    History::Events events;
    for (int i = 0; i < 50; ++i) {
        History::TextEvent textEvent(textThread.accountId(),
                                     textThread.threadId(),
                                     QString("eventId%1").arg(i),
                                     textParticipant,
                                     QDateTime::currentDateTime(),
                                     QDateTime::currentDateTime().addSecs(-10),
                                     true,
                                     QString("Hello world %1").arg(i),
                                     History::MessageTypeText,
                                     History::MessageStatusAccepted);
        events.append(textEvent);

        History::VoiceEvent voiceEvent(voiceThread.accountId(),
                                       voiceThread.threadId(),
                                       QString("eventId%1").arg(i),
                                       voiceParticipant,
                                       QDateTime::currentDateTime(),
                                       true,
                                       true);
        events.append(voiceEvent);
    }

    QSignalSpy newEventsSpy(mManager, SIGNAL(eventsAdded(History::Events)));
    QSignalSpy threadsModifiedSpy(mManager, SIGNAL(threadsModified(History::Threads)));
    QVERIFY(mManager->writeEvents(events));
    QTRY_COMPARE(newEventsSpy.count(), 1);
    QTRY_COMPARE(threadsModifiedSpy.count(), 1);

    // check that the signal was emitted with the correct number of events
    History::Events returnedEvents = newEventsSpy.first().first().value<History::Events>();

    // get two events to modify before sorting
    History::TextEvent modifiedTextEvent = events[0];
    History::VoiceEvent modifiedVoiceEvent = events[1];

    // just in case, sort the lists before comparing
    qSort(events);
    qSort(returnedEvents);
    QCOMPARE(returnedEvents, events);

    History::Threads returnedThreads = threadsModifiedSpy.first().first().value<History::Threads>();
    QCOMPARE(returnedThreads.count(), 2);

    // and now modify the events
    modifiedTextEvent.setNewEvent(false);
    modifiedTextEvent.setMessageStatus(History::MessageStatusDelivered);
    modifiedVoiceEvent.setNewEvent(false);

    QSignalSpy eventsModifiedSpy(mManager, SIGNAL(eventsModified(History::Events)));
    threadsModifiedSpy.clear();
    events.clear();
    events << modifiedTextEvent << modifiedVoiceEvent;
    QVERIFY(mManager->writeEvents(events));
    QTRY_COMPARE(eventsModifiedSpy.count(), 1);
    QTRY_COMPARE(threadsModifiedSpy.count(), 1);

    returnedEvents = eventsModifiedSpy.first().first().value<History::Events>();
    qDebug() << returnedEvents.first().accountId();
    QCOMPARE(returnedEvents.count(), 2);

    returnedThreads = threadsModifiedSpy.first().first().value<History::Threads>();
    QCOMPARE(returnedThreads.count(), 2);
}

void ManagerTest::testRemoveEvents()
{
    QString textParticipant("textParticipant");
    QString voiceParticipant("voiceParticipant");
    // create two threads, one for voice and one for text
    History::Thread textThread = mManager->threadForParticipants("textRemovableAccount",
                                                                 History::EventTypeText,
                                                                 QStringList()<< textParticipant,
                                                                 History::MatchCaseSensitive, true);
    History::Thread voiceThread = mManager->threadForParticipants("voiceRemovableAccount",
                                                                  History::EventTypeVoice,
                                                                  QStringList()<< voiceParticipant,
                                                                  History::MatchCaseSensitive, true);
    // insert some text and voice events
    History::Events events;
    for (int i = 0; i < 50; ++i) {
        History::TextEvent textEvent(textThread.accountId(),
                                     textThread.threadId(),
                                     QString("eventToBeRemoved%1").arg(i),
                                     textParticipant,
                                     QDateTime::currentDateTime(),
                                     QDateTime::currentDateTime().addSecs(-10),
                                     true,
                                     QString("Hello world %1").arg(i),
                                     History::MessageTypeText);
        events.append(textEvent);

        History::VoiceEvent voiceEvent(voiceThread.accountId(),
                                       voiceThread.threadId(),
                                       QString("eventToBeRemoved%1").arg(i),
                                       voiceParticipant,
                                       QDateTime::currentDateTime(),
                                       true,
                                       true);
        events.append(voiceEvent);
    }
    QSignalSpy eventsRemovedSpy(mManager, SIGNAL(eventsRemoved(History::Events)));
    QSignalSpy threadsModifiedSpy(mManager, SIGNAL(threadsModified(History::Threads)));
    QVERIFY(mManager->writeEvents(events));
    QTRY_COMPARE(threadsModifiedSpy.count(), 1);
    threadsModifiedSpy.clear();

    History::Events secondRemoval;
    secondRemoval << events.takeFirst() << events.takeLast();

    QVERIFY(mManager->removeEvents(events));
    QTRY_COMPARE(eventsRemovedSpy.count(), 1);
    QTRY_COMPARE(threadsModifiedSpy.count(), 1);
    History::Events removedEvents = eventsRemovedSpy.first().first().value<History::Events>();
    History::Threads modifiedThreads = threadsModifiedSpy.first().first().value<History::Threads>();
    qSort(events);
    qSort(removedEvents);
    QCOMPARE(removedEvents, events);
    QCOMPARE(modifiedThreads.count(), 2);

    // now remove the remaining events and make sure the threads get removed too
    QSignalSpy threadsRemovedSpy(mManager, SIGNAL(threadsRemoved(History::Threads)));
    eventsRemovedSpy.clear();
    QVERIFY(mManager->removeEvents(secondRemoval));
    QTRY_COMPARE(eventsRemovedSpy.count(), 1);
    QTRY_COMPARE(threadsRemovedSpy.count(), 1);
    removedEvents = eventsRemovedSpy.first().first().value<History::Events>();
    History::Threads removedThreads = threadsRemovedSpy.first().first().value<History::Threads>();
    qSort(removedEvents);
    qSort(secondRemoval);
    QCOMPARE(removedEvents, secondRemoval);
    QCOMPARE(removedThreads.count(), 2);
}

void ManagerTest::testRemoveEventsByFilter()
{

    QString textParticipant("textParticipant");
    QString voiceParticipant("voiceParticipant");

    QDateTime currentDate = QDateTime::currentDateTime();
    // create two threads, one for voice and one for text
    History::Thread textThread = mManager->threadForParticipants("textRemovableAccount",
                                                                 History::EventTypeText,
                                                                 QStringList()<< textParticipant,
                                                                 History::MatchCaseSensitive, true);
    History::Thread voiceThread = mManager->threadForParticipants("voiceRemovableAccount",
                                                                  History::EventTypeVoice,
                                                                  QStringList()<< voiceParticipant,
                                                                  History::MatchCaseSensitive, true);
    // insert some text and voice events
    History::Events events;
    for (int i = 0; i < 50; ++i) {
        History::TextEvent textEvent(textThread.accountId(),
                                     textThread.threadId(),
                                     QString("eventToBeRemoved%1").arg(i),
                                     textParticipant,
                                     currentDate.addDays(i),
                                     currentDate.addDays(i).addSecs(-10),
                                     true,
                                     QString("Hello world %1").arg(i),
                                     History::MessageTypeText);
        events.append(textEvent);


        History::VoiceEvent voiceEvent(voiceThread.accountId(),
                                       voiceThread.threadId(),
                                       QString("eventToBeRemoved%1").arg(i),
                                       voiceParticipant,
                                       currentDate.addDays(i),
                                       true,
                                       true);
        events.append(voiceEvent);
    }
    QSignalSpy eventsRemovedSpy(mManager, SIGNAL(eventsRemoved(History::Events)));
    QSignalSpy threadsModifiedSpy(mManager, SIGNAL(threadsModified(History::Threads)));
    QVERIFY(mManager->writeEvents(events));
    QTRY_COMPARE(threadsModifiedSpy.count(), 1);
    threadsModifiedSpy.clear();

    History::Filter filter;
    filter.setFilterProperty(History::FieldTimestamp);
    filter.setFilterValue(currentDate.addDays(10).toString("yyyy-MM-ddTHH:mm:ss.zzz")); //should delete 10 voice_events
    filter.setMatchFlags(History::MatchLess);

    History::Sort sort;
    sort.setSortField(History::FieldTimestamp);
    sort.setSortOrder(Qt::DescendingOrder);

    QVERIFY(mManager->removeEvents(History::EventTypeVoice ,filter, sort));
    QTRY_COMPARE(eventsRemovedSpy.count(), 1);
    QTRY_COMPARE(threadsModifiedSpy.count(), 1);
    History::Events removedEvents = eventsRemovedSpy.first().first().value<History::Events>();
    History::Threads modifiedThreads = threadsModifiedSpy.first().first().value<History::Threads>();

    QCOMPARE(removedEvents.count(), 10);
    QCOMPARE(modifiedThreads.count(), 1);

    // now remove the remaining events and make sure the threads get removed too
    filter.setFilterValue(currentDate.addYears(1).toString("yyyy-MM-ddTHH:mm:ss.zzz"));
    QSignalSpy threadsRemovedSpy(mManager, SIGNAL(threadsRemoved(History::Threads)));
    eventsRemovedSpy.clear();

    QVERIFY(mManager->removeEvents(History::EventTypeVoice ,filter, sort));
    QTRY_COMPARE(eventsRemovedSpy.count(), 1);
    QTRY_COMPARE(threadsRemovedSpy.count(), 1);
    removedEvents = eventsRemovedSpy.first().first().value<History::Events>();

    History::Threads removedThreads = threadsRemovedSpy.first().first().value<History::Threads>();

    QCOMPARE(removedEvents.count(), 40);
    QCOMPARE(removedThreads.count(), 1);

    //verify text events are still there
    QCOMPARE(mManager->eventsCount(History::EventTypeText ,filter), 50);

}

void ManagerTest::testGetSingleEvent()
{
    QString textParticipant("textSingleParticipant");
    QString voiceParticipant("voiceSingleParticipant");
    // create two threads, one for voice and one for text
    History::Thread textThread = mManager->threadForParticipants("textSingleAccount",
                                                                 History::EventTypeText,
                                                                 QStringList()<< textParticipant,
                                                                 History::MatchCaseSensitive, true);
    History::Thread voiceThread = mManager->threadForParticipants("voiceSingleAccount",
                                                                  History::EventTypeVoice,
                                                                  QStringList()<< voiceParticipant,
                                                                  History::MatchCaseSensitive, true);

    // now add two events
    History::TextEvent textEvent(textThread.accountId(),
                                 textThread.threadId(),
                                 "singleEventId",
                                 "self",
                                 QDateTime::currentDateTime(),
                                 QDateTime::currentDateTime().addSecs(-10),
                                 true,
                                 "Hello big world!",
                                 History::MessageTypeText,
                                 History::MessageStatusPending);
    History::VoiceEvent voiceEvent(voiceThread.accountId(),
                                   voiceThread.threadId(),
                                   "singleEventId",
                                   "self",
                                   QDateTime::currentDateTime(),
                                   false,
                                   false,
                                   QTime(1,2,3));
    QVERIFY(mManager->writeEvents(History::Events() << textEvent << voiceEvent));

    // and now try to get them
    History::TextEvent retrievedTextEvent = mManager->getSingleEvent(History::EventTypeText,
                                                                     textEvent.accountId(),
                                                                     textEvent.threadId(),
                                                                     textEvent.eventId());
    QVERIFY(retrievedTextEvent == textEvent);
    QCOMPARE(retrievedTextEvent.newEvent(), textEvent.newEvent());
    QCOMPARE(retrievedTextEvent.message(), textEvent.message());
    QCOMPARE(retrievedTextEvent.sentTime(), textEvent.sentTime());
    QCOMPARE(retrievedTextEvent.messageType(), textEvent.messageType());
    QCOMPARE(retrievedTextEvent.messageStatus(), textEvent.messageStatus());

    History::VoiceEvent retrievedVoiceEvent = mManager->getSingleEvent(History::EventTypeVoice,
                                                                       voiceEvent.accountId(),
                                                                       voiceEvent.threadId(),
                                                                       voiceEvent.eventId());
    QVERIFY(retrievedVoiceEvent == voiceEvent);
    QCOMPARE(retrievedVoiceEvent.newEvent(), voiceEvent.newEvent());
    QCOMPARE(retrievedVoiceEvent.missed(), voiceEvent.missed());
    QCOMPARE(retrievedVoiceEvent.duration(), voiceEvent.duration());
}

void ManagerTest::testRemoveThreads()
{
    // create two threads, one for voice and one for text
    History::Thread textThread = mManager->threadForParticipants("textThreadRemovalAccount",
                                                                 History::EventTypeText,
                                                                 QStringList()<< "textParticipant",
                                                                 History::MatchCaseSensitive, true);
    QVERIFY(!textThread.isNull());
    History::Thread voiceThread = mManager->threadForParticipants("voiceThreadRemovalAccount",
                                                                  History::EventTypeVoice,
                                                                  QStringList()<< "voiceParticipant",
                                                                  History::MatchCaseSensitive, true);
    QVERIFY(!voiceThread.isNull());

    History::Threads threads;
    threads << textThread << voiceThread;

    QSignalSpy threadsRemovedSpy(mManager, SIGNAL(threadsRemoved(History::Threads)));

    QVERIFY(mManager->removeThreads(threads));
    QTRY_COMPARE(threadsRemovedSpy.count(), 1);

    History::Threads removedThreads = threadsRemovedSpy.first().first().value<History::Threads>();
    qSort(removedThreads);
    qSort(threads);
    QCOMPARE(removedThreads, threads);
}

void ManagerTest::cleanup() {

    History::Filter filter;
    filter.setFilterProperty(History::FieldTimestamp);
    filter.setFilterValue(QDateTime::currentDateTime().addYears(10).toString("yyyy-MM-ddTHH:mm:ss.zzz"));
    filter.setMatchFlags(History::MatchLess);

    History::Sort sort;
    sort.setSortField(History::FieldTimestamp);
    sort.setSortOrder(Qt::DescendingOrder);

    int voiceEventsCount = mManager->eventsCount(History::EventTypeVoice, filter);
    int textEventsCount = mManager->eventsCount(History::EventTypeText, filter);

    int totalToRemove = voiceEventsCount + textEventsCount;
    int deletedCount = 0;
    if (totalToRemove > 0) {

        QMetaObject::Connection conn = connect(mManager,
                &History::Manager::eventsRemoved,
                [&deletedCount](const History::Events &events)
            {
                deletedCount+= events.count();
            });

        QVERIFY(mManager->removeEvents(History::EventTypeVoice, filter, sort));
        QVERIFY(mManager->removeEvents(History::EventTypeText, filter, sort));

        while (totalToRemove > deletedCount) {
            QTest::qWait(100);
        }

        QObject::disconnect(conn);
    }
}

void ManagerTest::cleanupTestCase()
{
    delete mManager;
}

QTEST_MAIN(ManagerTest)
#include "ManagerTest.moc"

