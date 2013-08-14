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

class ThreadWrapper : public History::Thread
{
public:
    ThreadWrapper(const QString &accountId,
                  const QString &threadId,
                  History::EventType type,
                  const QStringList &participants,
                  const History::EventPtr &lastEvent,
                  int count,
                  int unreadCount)
        : History::Thread(accountId, threadId, type, participants, lastEvent, count, unreadCount) { }
};

class ThreadTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCreateNewThread_data();
    void testCreateNewThread();
};

void ThreadTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
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

    ThreadWrapper threadItem(accountId, threadId, type, participants, event, count, unreadCount);
    QCOMPARE(threadItem.accountId(), accountId);
    QCOMPARE(threadItem.threadId(), threadId);
    QCOMPARE(threadItem.type(), type);
    QCOMPARE(threadItem.participants(), participants);
    QCOMPARE(threadItem.lastEvent(), event);
    QCOMPARE(threadItem.count(), count);
    QCOMPARE(threadItem.unreadCount(), unreadCount);

    QVariantMap properties = threadItem.properties();
    QCOMPARE(properties["accountId"].toString(), accountId);
    QCOMPARE(properties["threadId"].toString(), threadId);
    QCOMPARE(properties["type"].toInt(), (int)type);
    QCOMPARE(properties["participants"].toStringList(), participants);
    QCOMPARE(properties["count"].toInt(), count);
    QCOMPARE(properties["unreadCount"].toInt(), unreadCount);
}

QTEST_MAIN(ThreadTest)
#include "ThreadTest.moc"
