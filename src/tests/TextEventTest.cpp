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

class TextEventTest : public QObject
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
    void testSetProperties();
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
    QTest::addColumn<QString>("subject");

    QTest::newRow("unread message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject";
    QTest::newRow("read message") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-10)
                                    << false << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject 2";
    QTest::newRow("message flags") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << (int)(History::MessageFlagDelivered | History::MessageFlagPending)
                                    << QDateTime::currentDateTime().addDays(-5) << "Test Subject 3";
    QTest::newRow("multi party message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeMultiParty
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << QString();
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
    QFETCH(QString, subject);
    History::TextEvent event(accountId, threadId, eventId, senderId, timestamp, newEvent,
                             message, (History::MessageType)messageType, (History::MessageFlags)messageFlags,
                             readTimestamp, subject);

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
    QCOMPARE(event.subject(), subject);
}

void TextEventTest::testCastToEventAndBack()
{
    History::TextEvent textEvent("oneAccountId", "oneThreadId", "oneEventId", "oneSender", QDateTime::currentDateTime(),
                                 true, "Hello", History::MessageTypeText, 0, QDateTime());

    // test the copy constructor
    History::Event historyEvent(textEvent);
    QVERIFY(historyEvent == textEvent);
    History::TextEvent castBack(historyEvent);
    QVERIFY(castBack == textEvent);

    // and now the assignment operator
    History::Event anotherEvent;
    anotherEvent = textEvent;
    QVERIFY(anotherEvent == textEvent);
    History::TextEvent backAgain;
    backAgain = anotherEvent;
    QVERIFY(backAgain == textEvent);
}

void TextEventTest::testFromProperties_data()
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
    QTest::addColumn<QString>("subject");

    QTest::newRow("unread message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject";
    QTest::newRow("read message") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-10)
                                    << false << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject 2";
    QTest::newRow("message flags") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << (int)(History::MessageFlagDelivered | History::MessageFlagPending)
                                    << QDateTime::currentDateTime().addDays(-5) << "Test Subject 3";
    QTest::newRow("multi party message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeMultiParty
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << QString();
}

void TextEventTest::testFromProperties()
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
    QFETCH(QString, subject);

    QVariantMap properties;
    properties[History::FieldAccountId] = accountId;
    properties[History::FieldThreadId] = threadId;
    properties[History::FieldEventId] = eventId;
    properties[History::FieldSenderId] = senderId;
    properties[History::FieldTimestamp] = timestamp.toString(Qt::ISODate);
    properties[History::FieldNewEvent] = newEvent;
    properties[History::FieldMessage] = message;
    properties[History::FieldMessageType] = messageType;
    properties[History::FieldMessageFlags] = messageFlags;
    properties[History::FieldReadTimestamp] = readTimestamp.toString(Qt::ISODate);
    properties[History::FieldSubject] = subject;

    History::TextEvent textEvent = History::TextEvent::fromProperties(properties);
    QCOMPARE(textEvent.accountId(), accountId);
    QCOMPARE(textEvent.threadId(), threadId);
    QCOMPARE(textEvent.eventId(), eventId);
    QCOMPARE(textEvent.senderId(), senderId);
    QCOMPARE(textEvent.timestamp().toString(Qt::ISODate), timestamp.toString(Qt::ISODate));
    QCOMPARE(textEvent.newEvent(), newEvent);
    QCOMPARE(textEvent.message(), message);
    QCOMPARE(textEvent.messageType(), (History::MessageType) messageType);
    QCOMPARE(textEvent.messageFlags(), (History::MessageFlags) messageFlags);
    QCOMPARE(textEvent.readTimestamp().toString(Qt::ISODate), readTimestamp.toString(Qt::ISODate));
    QCOMPARE(textEvent.subject(), subject);
}

void TextEventTest::testFromNullProperties()
{
    // just to make sure, test that calling ::fromProperties() on an empty map returns a null event
    History::Event nullEvent = History::TextEvent::fromProperties(QVariantMap());
    QVERIFY(nullEvent.isNull());
    QCOMPARE(nullEvent.type(), History::EventTypeNull);
}

void TextEventTest::testProperties_data()
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
    QTest::addColumn<QString>("subject");

    QTest::newRow("unread message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject";
    QTest::newRow("read message") << "testAccountId2" << "testThreadId2" << "testEventId2"
                                    << "testSenderId2" << QDateTime::currentDateTime().addDays(-10)
                                    << false << "One Test Message" << (int)History::MessageTypeText
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << "Test Subject 2";
    QTest::newRow("message flags") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeText
                                    << (int)(History::MessageFlagDelivered | History::MessageFlagPending)
                                    << QDateTime::currentDateTime().addDays(-5) << "Test Subject 3";
    QTest::newRow("multi party message") << "testAccountId" << "testThreadId" << "testEventId"
                                    << "testSenderId" << QDateTime::currentDateTime().addDays(-10)
                                    << true << "One Test Message" << (int)History::MessageTypeMultiParty
                                    << 0 << QDateTime::currentDateTime().addDays(-5) << QString();
}

void TextEventTest::testProperties()
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
    QFETCH(QString, subject);
    History::TextEvent event(accountId, threadId, eventId, senderId, timestamp, newEvent,
                             message, (History::MessageType)messageType, (History::MessageFlags)messageFlags,
                             readTimestamp, subject);

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
    QCOMPARE(properties[History::FieldSubject].toString(), subject);
}

void TextEventTest::testSetProperties()
{
    History::TextEvent textEvent("oneAccountId", "oneThreadId", "oneEventId", "oneSender", QDateTime::currentDateTime(),
                                 true, "Hello", History::MessageTypeText, 0, QDateTime());
    QDateTime readTimestamp = QDateTime::currentDateTime();
    History::MessageFlags flags = History::MessageFlagDelivered;
    bool newEvent = false;
    textEvent.setReadTimestamp(readTimestamp);
    textEvent.setMessageFlags(flags);
    textEvent.setNewEvent(newEvent);
    QCOMPARE(textEvent.readTimestamp(), readTimestamp);
    QCOMPARE(textEvent.messageFlags(), flags);
    QCOMPARE(textEvent.newEvent(), newEvent);
}

QTEST_MAIN(TextEventTest)
#include "TextEventTest.moc"
