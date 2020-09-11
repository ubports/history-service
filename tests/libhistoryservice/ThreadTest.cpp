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

#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::Thread)

class ThreadTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCreateNewThread_data();
    void testCreateNewThread();
    void testFromProperties_data();
    void testFromProperties();
    void testFromNullProperties();
    void testProperties_data();
    void testProperties();
    void testIsNull_data();
    void testIsNull();
    void testEqualsOperator_data();
    void testEqualsOperator();
    void testCopyConstructor();
    void testAssignmentOperator();

private:
    History::Participants participantsFromIdentifiers(const QString &accountId, const QStringList &identifiers);
};

void ThreadTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::Thread>();
}

void ThreadTest::testCreateNewThread_data()
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

void ThreadTest::testCreateNewThread()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(History::EventType, type);
    QFETCH(QStringList, participants);
    QFETCH(int, count);
    QFETCH(int, unreadCount);

    History::Event event;
    switch (type) {
    case History::EventTypeText:
        // the eventId doesn´t really matter here, just faking a random one to not use always the same
        event = History::TextEvent(accountId, threadId, QString("eventId%1").arg(QString::number(qrand() % 1024)),
                                   participants[0], QDateTime::currentDateTime(), false, "Random Message",
                                   History::MessageTypeText);
        break;
    case History::EventTypeVoice:
        event = History::VoiceEvent(accountId, threadId, QString("eventId%1").arg(QString::number(qrand() % 1024)),
                                    participants[0], QDateTime::currentDateTime(), false, false, QTime(1,2,3));
        break;
    case History::EventTypeNull:
        break;
    }

    History::Thread threadItem(accountId, threadId, type, participantsFromIdentifiers(accountId, participants), event.timestamp(), event, count, unreadCount);
    QCOMPARE(threadItem.accountId(), accountId);
    QCOMPARE(threadItem.threadId(), threadId);
    QCOMPARE(threadItem.type(), type);
    QCOMPARE(threadItem.participants().identifiers(), participants);
    QCOMPARE(threadItem.timestamp(), event.timestamp());
    QCOMPARE(threadItem.lastEvent(), event);
    QCOMPARE(threadItem.count(), count);
    QCOMPARE(threadItem.unreadCount(), unreadCount);
    QVERIFY(threadItem.lastEvent() == event);
}

void ThreadTest::testFromProperties_data()
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

void ThreadTest::testFromProperties()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(History::EventType, type);
    QFETCH(QStringList, participants);
    QFETCH(int, count);
    QFETCH(int, unreadCount);

    History::Event event;
    switch (type) {
    case History::EventTypeText:
        // the eventId doesn´t really matter here, just faking a random one to not use always the same
        event = History::TextEvent(accountId, threadId, QString("eventId%1").arg(QString::number(qrand() % 1024)),
                                   participants[0], QDateTime::currentDateTime(), false, "Random Message",
                                   History::MessageTypeText);
        break;
    case History::EventTypeVoice:
        event = History::VoiceEvent(accountId, threadId, QString("eventId%1").arg(QString::number(qrand() % 1024)),
                                    participants[0], QDateTime::currentDateTime(), false, false, QTime(1,2,3));
        break;
    case History::EventTypeNull:
        break;
    }

    QVariantMap properties = event.properties();
    properties[History::FieldAccountId] = accountId;
    properties[History::FieldThreadId] = threadId;
    properties[History::FieldType] = (int) type;
    properties[History::FieldParticipants] = participantsFromIdentifiers(accountId, participants).toVariantList();
    properties[History::FieldCount] = count;
    properties[History::FieldUnreadCount] = unreadCount;

    History::Thread thread = History::Thread::fromProperties(properties);
    QCOMPARE(thread.accountId(), accountId);
    QCOMPARE(thread.threadId(), threadId);
    QCOMPARE(thread.type(), type);
    QCOMPARE(thread.participants().identifiers(), participants);
    QCOMPARE(thread.timestamp(), event.timestamp());
    QCOMPARE(thread.count(), count);
    QCOMPARE(thread.unreadCount(), unreadCount);
    QVERIFY(thread.lastEvent() == event);
}

void ThreadTest::testFromNullProperties()
{
    History::Thread thread = History::Thread::fromProperties(QVariantMap());
    QVERIFY(thread.isNull());
}

void ThreadTest::testProperties_data()
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

void ThreadTest::testProperties()
{
    QFETCH(QString, accountId);
    QFETCH(QString, threadId);
    QFETCH(History::EventType, type);
    QFETCH(QStringList, participants);
    QFETCH(int, count);
    QFETCH(int, unreadCount);

    History::Event event;
    switch (type) {
    case History::EventTypeText:
        // the eventId doesn´t really matter here, just faking a random one to not use always the same
        event = History::TextEvent(accountId, threadId, QString("eventId%1").arg(QString::number(qrand() % 1024)),
                                   participants[0], QDateTime::currentDateTime(), false, "Random Message",
                                   History::MessageTypeText);
        break;
    case History::EventTypeVoice:
        event = History::VoiceEvent(accountId, threadId, QString("eventId%1").arg(QString::number(qrand() % 1024)),
                                    participants[0], QDateTime::currentDateTime(), false, false, QTime(1,2,3));
        break;
    case History::EventTypeNull:
        break;
    }

    History::Thread threadItem(accountId, threadId, type, participantsFromIdentifiers(accountId, participants), event.timestamp(), event, count, unreadCount);
    QVariantMap properties = threadItem.properties();
    QCOMPARE(properties[History::FieldAccountId].toString(), accountId);
    QCOMPARE(properties[History::FieldThreadId].toString(), threadId);
    QCOMPARE(properties[History::FieldType].toInt(), (int)type);
    QCOMPARE(History::Participants::fromVariantList(properties[History::FieldParticipants].toList()).identifiers(), participants);
    QCOMPARE(properties[History::FieldCount].toInt(), count);
    QCOMPARE(properties[History::FieldUnreadCount].toInt(), unreadCount);
}

void ThreadTest::testIsNull_data()
{
    QTest::addColumn<History::Thread>("thread");
    QTest::addColumn<bool>("isNull");

    History::Participants participants;
    participants << History::Participant("AccountId","Foo") << History::Participant("AccountId","Bar");

    QTest::newRow("empty thread") << History::Thread() << true;
    QTest::newRow("empty accountId") << History::Thread(QString(), "threadId" , History::EventTypeText, participants) << false;
    QTest::newRow("empty threadId") << History::Thread("AccountId", QString(), History::EventTypeVoice, participants) << false;
    QTest::newRow("empty participants") << History::Thread("AccountId", "ThreadId", History::EventTypeText, History::Participants()) << false;
    QTest::newRow("construct empty thread") << History::Thread(QString(), QString(), History::EventTypeNull, History::Participants()) << true;
}

void ThreadTest::testIsNull()
{
    QFETCH(History::Thread, thread);
    QFETCH(bool, isNull);
    QCOMPARE(thread.isNull(), isNull);
}

void ThreadTest::testEqualsOperator_data()
{
    QTest::addColumn<QString>("firstAccountId");
    QTest::addColumn<QString>("firstThreadId");
    QTest::addColumn<History::EventType>("firstType");
    QTest::addColumn<QString>("secondAccountId");
    QTest::addColumn<QString>("secondThreadId");
    QTest::addColumn<History::EventType>("secondType");
    QTest::addColumn<bool>("result");


    QTest::newRow("equal threads") << "theAccountId" << "theThreadId" << History::EventTypeText
                                   << "theAccountId" << "theThreadId" << History::EventTypeText << true;
    QTest::newRow("different types") << "oneAccountId" << "oneThreadId" << History::EventTypeVoice
                                     << "oneAccountId" << "oneThreadId" << History::EventTypeText << false;
    QTest::newRow("different account IDs") << "firstAccountId" << "theThreadId" << History::EventTypeVoice
                                           << "secondAccountId" << "theThreadId" << History::EventTypeVoice << false;
    QTest::newRow("different thread IDs") << "oneAccountId" << "firstThreadId" << History::EventTypeText
                                          << "oneAccountId" << "secondThreadId" << History::EventTypeText << false;
}

void ThreadTest::testEqualsOperator()
{
    QFETCH(QString, firstAccountId);
    QFETCH(QString, firstThreadId);
    QFETCH(History::EventType, firstType);
    QFETCH(QString, secondAccountId);
    QFETCH(QString, secondThreadId);
    QFETCH(History::EventType, secondType);
    QFETCH(bool, result);

    History::Thread firstThread(firstAccountId, firstThreadId, firstType, History::Participants());
    History::Thread secondThread(secondAccountId, secondThreadId, secondType, History::Participants());
    QVERIFY((firstThread == secondThread) == result);
}

void ThreadTest::testCopyConstructor()
{
    History::Thread thread("OneAccountId", "OneThreadId", History::EventTypeText, participantsFromIdentifiers("OneAccountId", QStringList() << "Foo" << "Bar"));
    History::Thread copy(thread);
    QVERIFY(thread == copy);
}

void ThreadTest::testAssignmentOperator()
{
    History::Thread thread("OneAccountId", "OneThreadId", History::EventTypeText, participantsFromIdentifiers("OneAccountId", QStringList() << "Foo" << "Bar"));
    History::Thread other;
    other = thread;
    QVERIFY(other == thread);
}

History::Participants ThreadTest::participantsFromIdentifiers(const QString &accountId, const QStringList &identifiers)
{
    History::Participants participants;
    Q_FOREACH(const QString &identifier, identifiers) {
        participants << History::Participant(accountId, identifier);
    }
    return participants;
}

QTEST_MAIN(ThreadTest)
#include "ThreadTest.moc"
