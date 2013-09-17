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

#include "textevent.h"

class TextEventWrapper : public History::TextEvent
{
public:
    TextEventWrapper(const QString &accountId,
                     const QString &threadId,
                     const QString &eventId,
                     const QString &senderId,
                     const QDateTime &timestamp,
                     bool newEvent,
                     const QString &message,
                     History::MessageType messageType,
                     History::MessageFlags messageFlags,
                     const QDateTime &readTimestamp)
    : TextEvent(accountId, threadId, eventId, senderId, timestamp, newEvent,
                message, messageType, messageFlags, readTimestamp) { }
};

class TextEventTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCreateNewEvent_data();
    void testCreateNewEvent();
};

void TextEventTest::testCreateNewEvent_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("threadId");
    QTest::addColumn<QString>("eventId");
    QTest::addColumn<QString>("senderId");
    QTest::addColumn<QDateTime>("timestamp");
    QTest::addColumn<bool>("newEvent");
    QTest::addColumn<QString>("message");
    QTest::addColumn<int>("messageType");
    QTest::addColumn<int>("messageFlags");
    QTest::addColumn<QDateTime>("readTimestamp");

    QTest::newRow("unread message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5);
    QTest::newRow("read message") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-10)
                                    << false << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5);
    QTest::newRow("message flags") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << (int)(History::MessageFlagDelivered | History::MessageFlagPending)
                                    << QDateTime::currentDateTime().addDays(-5);
    QTest::newRow("multi party message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeMultiParty
                                    << 0 << QDateTime::currentDateTime().addDays(-5);
}

void TextEventTest::testCreateNewEvent()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(QString, eventId);
    QFETCH(QString, senderId);
    QFETCH(QDateTime, timestamp);
    QFETCH(bool, newEvent);
    QFETCH(QString, message);
    QFETCH(int, messageType);
    QFETCH(int, messageFlags);
    QFETCH(QDateTime, readTimestamp);
    TextEventWrapper event(accountId, threadId, eventId, senderId, timestamp, newEvent,
                           message, (History::MessageType)messageType, (History::MessageFlags)messageFlags,
                           readTimestamp);

    // check that the values are properly set
    QCOMPARE(event.accountId(), accountId);
    QCOMPARE(event.threadId(), threadId);
    QCOMPARE(event.eventId(), eventId);
    QCOMPARE(event.senderId(), senderId);
    QCOMPARE(event.timestamp(), timestamp);
    QCOMPARE(event.newEvent(), newEvent);
    QCOMPARE(event.message(), message);
    QCOMPARE(event.messageType(), (History::MessageType)messageType);
    QCOMPARE(event.messageFlags(), (History::MessageFlags)messageFlags);
    QCOMPARE(event.readTimestamp(), readTimestamp);

    QVariantMap properties = event.properties();
    QCOMPARE(properties[History::FieldAccountId].toString(), accountId);
    QCOMPARE(properties[History::FieldThreadId].toString(), threadId);
    QCOMPARE(properties[History::FieldEventId].toString(), eventId);
    QCOMPARE(properties[History::FieldSenderId].toString(), senderId);
    QCOMPARE(properties[History::FieldTimestamp].toString(), timestamp.toString(Qt::ISODate));
    QCOMPARE(properties[History::FieldNewEvent].toBool(), newEvent);
    QCOMPARE(properties[History::FieldMessage].toString(), message);
    QCOMPARE(properties[History::FieldMessageType].toInt(), messageType);
    QCOMPARE(properties[History::FieldMessageFlags].toInt(), messageFlags);
    QCOMPARE(properties[History::FieldReadTimestamp].toString(), readTimestamp.toString(Qt::ISODate));
}

QTEST_MAIN(TextEventTest)
#include "TextEventTest.moc"
