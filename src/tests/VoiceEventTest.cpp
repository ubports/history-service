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

class VoiceEventWrapper : public History::VoiceEvent
{
public:
    VoiceEventWrapper(const QString &accountId,
                      const QString &threadId,
                      const QString &eventId,
                      const QString &senderId,
                      const QDateTime &timestamp,
                      bool newEvent,
                      bool missed,
                      const QTime &duration)
    : VoiceEvent(accountId, threadId, eventId, senderId, timestamp, newEvent,
                 missed, duration) { }
};

class VoiceEventTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreateNewEvent_data();
    void testCreateNewEvent();
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
    VoiceEventWrapper event(accountId, threadId, eventId, senderId, timestamp, newEvent,
                            missed, duration);

    // check that the values are properly set
    QCOMPARE(event.accountId(), accountId);
    QCOMPARE(event.threadId(), threadId);
    QCOMPARE(event.eventId(), eventId);
    QCOMPARE(event.senderId(), senderId);
    QCOMPARE(event.timestamp(), timestamp);
    QCOMPARE(event.newEvent(), newEvent);
    QCOMPARE(event.missed(), missed);
    QCOMPARE(event.duration(), duration);

    QVariantMap properties = event.properties();
    QCOMPARE(properties["accountId"].toString(), accountId);
    QCOMPARE(properties["threadId"].toString(), threadId);
    QCOMPARE(properties["eventId"].toString(), eventId);
    QCOMPARE(properties["senderId"].toString(), senderId);
    QCOMPARE(properties["timestamp"].toDateTime(), timestamp);
    QCOMPARE(properties["newEvent"].toBool(), newEvent);
    QCOMPARE(properties["missed"].toBool(), missed);
    QCOMPARE(properties["duration"].toTime(), duration);
}

QTEST_MAIN(VoiceEventTest)
#include "VoiceEventTest.moc"
