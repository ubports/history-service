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

#include "voiceevent.h"

class VoiceEventTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreateNewEvent_data();
    void testCreateNewEvent();
    void testCastToEventAndBack();
    void testFromProperties_data();
    void testFromProperties();
    void testFromNullProperties();
    void testProperties_data();
    void testProperties();
};

void VoiceEventTest::testCreateNewEvent_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("senderId");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<bool>("missed");
    QTest::addColumn<QTime>("duration");
    QTest::addColumn<QString>("remoteParticipant");
    QTest::addColumn<QStringList>("participants");

    QTest::newRow("unread missed call") << "testAccountId" << "testThreadId" << "testEventId"
                                 << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                 << true << true << QTime(0, 0, 0) << QString("remoteParticipant")
                                 << (QStringList() << "testSenderId");
    QTest::newRow("missed call") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                 << "testSenderId2" << QDateTime::currentDateTime().addDays(-5)
                                 << false << true << QTime(0, 0, 0) << QString("remoteParticipant2")
                                 << (QStringList() << "testSenderId2");
    QTest::newRow("not missed call") << "testAccountId" << "testThreadId" << "testEventId"
                                 << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                 << false << false << QTime(1, 2, 3) << QString("remoteParticipant")
                                 << (QStringList() << "testSenderId");
}

void VoiceEventTest::testCreateNewEvent()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, senderId);
    QFETCH(QDateTime, timestamp);
    QFETCH(bool, newEvent);
    QFETCH(bool, missed);
    QFETCH(QTime, duration);
    QFETCH(QString, remoteParticipant);
    QFETCH(QStringList, participants);
    History::VoiceEvent event(accountId, threadId, eventId, senderId, timestamp, newEvent,
                              missed, duration, remoteParticipant, participants);

    // check that the values are properly set
    QCOMPARE(event.accountId(), accountId);
    QCOMPARE(event.threadId(), threadId);
    QCOMPARE(event.eventId(), eventId);
    QCOMPARE(event.senderId(), senderId);
    QCOMPARE(event.timestamp(), timestamp);
    QCOMPARE(event.newEvent(), newEvent);
    QCOMPARE(event.missed(), missed);
    QCOMPARE(event.duration(), duration);
    QCOMPARE(event.remoteParticipant(), remoteParticipant);
    QCOMPARE(event.participants(), participants);
}

void VoiceEventTest::testCastToEventAndBack()
{
    History::VoiceEvent voiceEvent("oneAccountId", "oneThreadId", "oneEventId", "oneSender", QDateTime::currentDateTime(),
                                   true, true, QTime(1,2,3), "remoteParticipant", QStringList() << "oneParticipant");

    // test the copy constructor
    History::Event historyEvent(voiceEvent);
    QVERIFY(historyEvent == voiceEvent);
    History::VoiceEvent castBack(historyEvent);
    QVERIFY(castBack == voiceEvent);

    // and now the assignment operator
    History::Event anotherEvent;
    anotherEvent = voiceEvent;
    QVERIFY(anotherEvent == voiceEvent);
    History::VoiceEvent backAgain;
    backAgain = anotherEvent;
    QVERIFY(backAgain == voiceEvent);
}

void VoiceEventTest::testFromProperties_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("senderId");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<bool>("missed");
    QTest::addColumn<QTime>("duration");
    QTest::addColumn<QStringList>("participants");

    QTest::newRow("unread missed call") << "testAccountId" << "testThreadId" << "testEventId"
                                 << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                 << true << true << QTime(0, 0, 0) << (QStringList() << "testParticipant");
    QTest::newRow("missed call") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                 << "testSenderId2" << QDateTime::currentDateTime().addDays(-5)
                                 << false << true << QTime(0, 0, 0) << (QStringList() << "testParticipant2");
    QTest::newRow("not missed call") << "testAccountId" << "testThreadId" << "testEventId"
                                 << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                 << false << false << QTime(1, 2, 3) << (QStringList() << "testParticipant");
}

void VoiceEventTest::testFromProperties()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, senderId);
    QFETCH(QDateTime, timestamp);
    QFETCH(bool, newEvent);
    QFETCH(bool, missed);
    QFETCH(QTime, duration);
    QFETCH(QStringList, participants);

    QVariantMap properties;
    properties[History::FieldAccountId] = accountId;
    properties[History::FieldThreadId] = threadId;
    properties[History::FieldEventId] = eventId;
    properties[History::FieldSenderId] = senderId;
    properties[History::FieldTimestamp] = timestamp.toString(Qt::ISODate);
    properties[History::FieldNewEvent] = newEvent;
    properties[History::FieldMissed] = missed;
    properties[History::FieldDuration] = QTime(0,0,0,0).secsTo(duration);
    properties[History::FieldParticipants] = participants;

    History::VoiceEvent voiceEvent = History::VoiceEvent::fromProperties(properties);
    QCOMPARE(voiceEvent.accountId(), accountId);
    QCOMPARE(voiceEvent.threadId(), threadId);
    QCOMPARE(voiceEvent.eventId(), eventId);
    QCOMPARE(voiceEvent.senderId(), senderId);
    QCOMPARE(voiceEvent.timestamp().toString(Qt::ISODate), timestamp.toString(Qt::ISODate));
    QCOMPARE(voiceEvent.newEvent(), newEvent);
    QCOMPARE(voiceEvent.missed(), missed);
    QCOMPARE(voiceEvent.duration(), duration);
    QCOMPARE(voiceEvent.participants(), participants);
}

void VoiceEventTest::testFromNullProperties()
{
    // just to make sure, test that calling ::fromProperties() on an empty map returns a null event
    History::Event nullEvent = History::VoiceEvent::fromProperties(QVariantMap());
    QVERIFY(nullEvent.isNull());
    QCOMPARE(nullEvent.type(), History::EventTypeNull);
}

void VoiceEventTest::testProperties_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("senderId");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<bool>("missed");
    QTest::addColumn<QTime>("duration");
    QTest::addColumn<QString>("remoteParticipant");
    QTest::addColumn<QStringList>("participants");

    QTest::newRow("unread missed call") << "testAccountId" << "testThreadId" << "testEventId"
                                 << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                 << true << true << QTime(0, 0, 0) << QString("remoteParticipant")
                                 << (QStringList() << "testParticipant");
    QTest::newRow("missed call") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                 << "testSenderId2" << QDateTime::currentDateTime().addDays(-5)
                                 << false << true << QTime(0, 0, 0) << QString("remoteParticipant2")
                                 << (QStringList() << "testParticipant2");
    QTest::newRow("not missed call") << "testAccountId" << "testThreadId" << "testEventId"
                                 << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                 << false << false << QTime(1, 2, 3) << QString("remoteParticipant3")
                                 << (QStringList() << "testParticipant3");
}

void VoiceEventTest::testProperties()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, senderId);
    QFETCH(QDateTime, timestamp);
    QFETCH(bool, newEvent);
    QFETCH(bool, missed);
    QFETCH(QTime, duration);
    QFETCH(QString, remoteParticipant);
    QFETCH(QStringList, participants);
    History::VoiceEvent event(accountId, threadId, eventId, senderId, timestamp, newEvent,
                              missed, duration, remoteParticipant, participants);

    // check that the values are properly set
    QVariantMap properties = event.properties();
    QCOMPARE(properties[History::FieldAccountId].toString(), accountId);
    QCOMPARE(properties[History::FieldThreadId].toString(), threadId);
    QCOMPARE(properties[History::FieldEventId].toString(), eventId);
    QCOMPARE(properties[History::FieldSenderId].toString(), senderId);
    QCOMPARE(properties[History::FieldTimestamp].toString(), timestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz"));
    QCOMPARE(properties[History::FieldNewEvent].toBool(), newEvent);
    QCOMPARE(properties[History::FieldMissed].toBool(), missed);
    QCOMPARE(QTime(0,0).addSecs(properties[History::FieldDuration].toInt()), duration);
    QCOMPARE(properties[History::FieldRemoteParticipant].toString(), remoteParticipant);
    QCOMPARE(properties[History::FieldParticipants].toStringList(), participants);
}

QTEST_MAIN(VoiceEventTest)
#include "VoiceEventTest.moc"
