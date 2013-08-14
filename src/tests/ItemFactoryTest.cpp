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

#include "itemfactory.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::MessageType)
Q_DECLARE_METATYPE(History::MessageFlags)

class ItemFactoryTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCreateNewThread_data();
    void testCreateNewThread();
    void testCreateNewTextEvent_data();
    void testCreateNewTextEvent();
    void testCreateNewVoiceEvent_data();
    void testCreateNewVoiceEvent();
    void testUseCacheWhenRecreatingThread();
    void testUseCacheWhenRecreatingTextEvent();
    void testUseCacheWhenRecreatingVoiceEvent();
    void testCachedThread();
    void testCachedEvent();
};

void ItemFactoryTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::MessageType>();
    qRegisterMetaType<History::MessageFlags>();
}

void ItemFactoryTest::testCreateNewThread_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<History::EventType>("type");
    QTest::addColumn<QStringList>("participants");
    QTest::addColumn<int>("count");
    QTest::addColumn<int>("unreadCount");

    QTest::newRow("voice thread with count, unread count and one participant")
             << "someaccountid" << "somethreadid" << History::EventTypeVoice
             << (QStringList() << "someparticipant") << 10 << 4;
    QTest::newRow("voice thread with zero messages and two participants")
             << "anotheraccountid" << "anotherthreadid" << History::EventTypeVoice
             << (QStringList() << "someparticipant" << "anotherparticipant") << 0 << 0;
    QTest::newRow("text thread with count, unread count and one participant")
             << "somevoiceaccountid" << "somevoicethreadid" << History::EventTypeText
             << (QStringList() << "somevoiceparticipant") << 10 << 4;
    QTest::newRow("voice thread with zero messages and two participants")
             << "anothervoiceaccountid" << "anothervoicethreadid" << History::EventTypeText
             << (QStringList() << "someparticipant" << "anotherparticipant") << 0 << 0;
}

void ItemFactoryTest::testCreateNewThread()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(History::EventType, type);
    QFETCH(QStringList, participants);
    QFETCH(int, count);
    QFETCH(int, unreadCount);

    History::EventPtr event;
    switch (type) {
    case History::EventTypeText:
        // the eventId doesn´t really matter here, just faking a random one to not use always the same
        event = History::ItemFactory::instance()->createTextEvent(accountId, threadId, QString("eventId%1").arg(QString::number(qrand() % 1024)),
                                                                  participants[0], QDateTime::currentDateTime(), false, "Random Message",
                                                                  History::MessageTypeText, History::MessageFlags(), QDateTime::currentDateTime());
        break;
    case History::EventTypeVoice:
        event = History::ItemFactory::instance()->createVoiceEvent(accountId, threadId, QString("eventId%1").arg(QString::number(qrand() % 1024)),
                                                                   participants[0], QDateTime::currentDateTime(), false, false, QTime(1,2,3));
        break;
    }

    History::ThreadPtr threadItem = History::ItemFactory::instance()->createThread(accountId, threadId, type, participants,
                                                                                   event, count, unreadCount);
    QVERIFY(!threadItem.isNull());
    QCOMPARE(threadItem->accountId(), accountId);
    QCOMPARE(threadItem->threadId(), threadId);
    QCOMPARE(threadItem->type(), type);
    QCOMPARE(threadItem->participants(), participants);
    QCOMPARE(threadItem->lastEvent(), event);
    QCOMPARE(threadItem->count(), count);
    QCOMPARE(threadItem->unreadCount(), unreadCount);
}

void ItemFactoryTest::testCreateNewTextEvent_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("senderId");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<QString>("message");
    QTest::addColumn<History::MessageType>("messageType");
    QTest::addColumn<History::MessageFlags>("messageFlags");
    QTest::addColumn<QDateTime>("readTimestamp");

    QTest::newRow("unread message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << History::MessageTypeText
                                    << History::MessageFlags() << QDateTime::currentDateTime().addDays(-5);
    QTest::newRow("read message") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-10)
                                    << false << "One Test Message" << History::MessageTypeText
                                    << History::MessageFlags() << QDateTime::currentDateTime().addDays(-5);
    QTest::newRow("message flags") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << History::MessageTypeText
                                    << (History::MessageFlagDelivered | History::MessageFlagPending)
                                    << QDateTime::currentDateTime().addDays(-5);
    QTest::newRow("multi party message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << History::MessageTypeMultiParty
                                    << History::MessageFlags() << QDateTime::currentDateTime().addDays(-5);
}

void ItemFactoryTest::testCreateNewTextEvent()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, senderId);
    QFETCH(QDateTime, timestamp);
    QFETCH(bool, newEvent);
    QFETCH(QString, message);
    QFETCH(History::MessageType, messageType);
    QFETCH(History::MessageFlags, messageFlags);
    QFETCH(QDateTime, readTimestamp);

    History::TextEventPtr textEvent = History::ItemFactory::instance()->createTextEvent(accountId, threadId, eventId,
                                                                                        senderId, timestamp, newEvent,
                                                                                        message, messageType, messageFlags, readTimestamp);

    // check that the values are properly set
    QVERIFY(!textEvent.isNull());
    QCOMPARE(textEvent->accountId(), accountId);
    QCOMPARE(textEvent->threadId(), threadId);
    QCOMPARE(textEvent->eventId(), eventId);
    QCOMPARE(textEvent->senderId(), senderId);
    QCOMPARE(textEvent->timestamp(), timestamp);
    QCOMPARE(textEvent->newEvent(), newEvent);
    QCOMPARE(textEvent->message(), message);
    QCOMPARE(textEvent->messageType(), (History::MessageType)messageType);
    QCOMPARE(textEvent->messageFlags(), (History::MessageFlags)messageFlags);
    QCOMPARE(textEvent->readTimestamp(), readTimestamp);
}

void ItemFactoryTest::testCreateNewVoiceEvent_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("senderId");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<bool>("missed");
    QTest::addColumn<QTime>("duration");

    QTest::newRow("unread missed call") << "testAccountId" << "testThreadId" << "testEventId"
                                 << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                 << true << true << QTime(0, 0, 0);
    QTest::newRow("missed call") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                 << "testSenderId2" << QDateTime::currentDateTime().addDays(-5)
                                 << false << true << QTime(0, 0, 0);
    QTest::newRow("not missed call") << "testAccountId" << "testThreadId" << "testEventId"
                                 << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                 << false << false << QTime(1, 2, 3);
}

void ItemFactoryTest::testCreateNewVoiceEvent()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, senderId);
    QFETCH(QDateTime, timestamp);
    QFETCH(bool, newEvent);
    QFETCH(bool, missed);
    QFETCH(QTime, duration);
    History::VoiceEventPtr voiceEvent = History::ItemFactory::instance()->createVoiceEvent(accountId, threadId, eventId,
                                                                                           senderId, timestamp, newEvent,
                                                                                           missed, duration);

    // check that the values are properly set
    QVERIFY(!voiceEvent.isNull());
    QCOMPARE(voiceEvent->accountId(), accountId);
    QCOMPARE(voiceEvent->threadId(), threadId);
    QCOMPARE(voiceEvent->eventId(), eventId);
    QCOMPARE(voiceEvent->senderId(), senderId);
    QCOMPARE(voiceEvent->timestamp(), timestamp);
    QCOMPARE(voiceEvent->newEvent(), newEvent);
    QCOMPARE(voiceEvent->missed(), missed);
    QCOMPARE(voiceEvent->duration(), duration);
}

void ItemFactoryTest::testUseCacheWhenRecreatingThread()
{
    QString accountId("someAccountId");
    QString threadId("someThreadId");
    QStringList participants;
    participants << "someParticipantId";
    History::EventType type(History::EventTypeVoice);
    int count(10);
    int unreadCount(5);
    History::ThreadPtr threadItem = History::ItemFactory::instance()->createThread(accountId, threadId, type, participants,
                                                                                   History::EventPtr(), count, unreadCount);

    QVERIFY(!threadItem.isNull());

    // now create a secondary thread with same accoundId, threadId and type to see if the one from the cache gets reused
    QString eventId("someEventId");
    History::EventPtr event = History::ItemFactory::instance()->createVoiceEvent(accountId, threadId, eventId,
                                                                                 participants[0], QDateTime::currentDateTime(), false, false, QTime(1,2,3));
    participants << "randomParticipantId";
    count = 7;
    unreadCount = 2;
    History::ThreadPtr anotherThread = History::ItemFactory::instance()->createThread(accountId, threadId, type, participants,
                                                                                      event, count, unreadCount);

    // check if the thread items are the same
    QVERIFY(!anotherThread.isNull());
    QCOMPARE(threadItem, anotherThread);

    // and check that the values got updated correctly
    QCOMPARE(anotherThread->accountId(), accountId);
    QCOMPARE(anotherThread->threadId(), threadId);
    QCOMPARE(anotherThread->type(), type);
    QCOMPARE(anotherThread->participants(), participants);
    QCOMPARE(anotherThread->lastEvent(), event);
    QCOMPARE(anotherThread->count(), count);
    QCOMPARE(anotherThread->unreadCount(), unreadCount);

    // and just to make sure, check that creating a thread with a different type returns a new item and not the cached one
    anotherThread = History::ItemFactory::instance()->createThread(accountId, threadId, History::EventTypeText, participants,
                                                                   History::EventPtr(), count, unreadCount);
    QVERIFY(!anotherThread.isNull());
    QVERIFY(anotherThread != threadItem);
}

void ItemFactoryTest::testUseCacheWhenRecreatingTextEvent()
{
    QString accountId("someAccountId");
    QString threadId("someThreadId");
    QString eventId("someEventId");
    QString senderId("someSenderId");
    QDateTime timestamp(QDateTime::currentDateTime());
    bool newEvent(false);
    QString message("Hello world!");
    History::MessageType messageType(History::MessageTypeText);
    History::MessageFlags messageFlags;
    QDateTime readTimestamp(QDateTime::currentDateTime());

    History::TextEventPtr textEvent = History::ItemFactory::instance()->createTextEvent(accountId, threadId, eventId, senderId, timestamp,
                                                                                        newEvent, message, messageType, messageFlags, readTimestamp);
    QVERIFY(!textEvent.isNull());


    // now change some values parameter to see if they get updated
    newEvent = true;
    messageFlags = History::MessageFlagDelivered;
    readTimestamp = QDateTime::currentDateTime().addDays(-4);
    History::TextEventPtr anotherTextEvent = History::ItemFactory::instance()->createTextEvent(accountId, threadId, eventId, senderId, timestamp,
                                                                                               newEvent, message, messageType, messageFlags, readTimestamp);

    // verify that the cached entry got reused and updated
    QVERIFY(!anotherTextEvent.isNull());
    QCOMPARE(anotherTextEvent, textEvent);
    QCOMPARE(anotherTextEvent->newEvent(), newEvent);
    QCOMPARE(anotherTextEvent->messageFlags(), messageFlags);
    QCOMPARE(anotherTextEvent->readTimestamp(), readTimestamp);
}

void ItemFactoryTest::testUseCacheWhenRecreatingVoiceEvent()
{
    QString accountId("someAccountId");
    QString threadId("someThreadId");
    QString eventId("someEventId");
    QString senderId("someSenderId");
    QDateTime timestamp(QDateTime::currentDateTime());
    bool newEvent(false);
    bool missed(false);
    QTime duration(1, 2, 3);

    History::VoiceEventPtr voiceEvent = History::ItemFactory::instance()->createVoiceEvent(accountId, threadId, eventId, senderId,
                                                                                           timestamp, newEvent, missed, duration);
    QVERIFY(!voiceEvent.isNull());


    // now change the newEvent parameter to see if it gets updated
    newEvent = true;
    History::VoiceEventPtr anotherVoiceEvent = History::ItemFactory::instance()->createVoiceEvent(accountId, threadId, eventId, senderId,
                                                                                                  timestamp, newEvent, missed, duration);

    // verify that the cached entry got reused and updated
    QVERIFY(!anotherVoiceEvent.isNull());
    QCOMPARE(anotherVoiceEvent, voiceEvent);
    QCOMPARE(anotherVoiceEvent->newEvent(), newEvent);
}

void ItemFactoryTest::testCachedThread()
{
    QString accountId("someAccountIdToCache");
    QString threadId("someThreadIdToCache");
    QStringList participants;
    participants << "someParticipantIdToCache";
    History::EventType type(History::EventTypeText);
    int count(4);
    int unreadCount(2);
    History::ThreadPtr threadItem = History::ItemFactory::instance()->createThread(accountId, threadId, type, participants,
                                                                                   History::EventPtr(), count, unreadCount);

    QVERIFY(!threadItem.isNull());

    // now get the cached thread
    History::ThreadPtr cachedThread = History::ItemFactory::instance()->cachedThread(accountId, threadId, type);
    QVERIFY(!cachedThread.isNull());
    QCOMPARE(cachedThread, threadItem);

    // now check that a thread of a different type is not returned
    History::ThreadPtr differentThread = History::ItemFactory::instance()->cachedThread(accountId, threadId, History::EventTypeVoice);
    QVERIFY(differentThread != threadItem);
}

void ItemFactoryTest::testCachedEvent()
{
    QString accountId("someAccountIdToCache");
    QString threadId("someThreadIdToCache");
    QString eventId("someEventIdToCache");
    QString senderId("someSenderIdToCache");
    QDateTime timestamp(QDateTime::currentDateTime());
    bool newEvent(false);
    bool missed(false);
    QTime duration(1, 2, 3);

    History::EventPtr event = History::ItemFactory::instance()->createVoiceEvent(accountId, threadId, eventId, senderId,
                                                                                 timestamp, newEvent, missed, duration);
    QVERIFY(!event.isNull());

    History::EventPtr cachedEvent = History::ItemFactory::instance()->cachedEvent(accountId, threadId, eventId, History::EventTypeVoice);
    QVERIFY(!cachedEvent.isNull());
    QCOMPARE(cachedEvent, event);

    // and check that an event of a different type is not returned
    History::EventPtr anotherEvent = History::ItemFactory::instance()->cachedEvent(accountId, threadId, eventId, History::EventTypeText);
    QVERIFY(anotherEvent != event);
}

QTEST_MAIN(ItemFactoryTest)
#include "ItemFactoryTest.moc"

