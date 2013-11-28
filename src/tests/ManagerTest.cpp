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
#include "manager.h"
#include "thread.h"
#include "threadview.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::MatchFlags)
Q_DECLARE_METATYPE(History::Threads)

class ManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testThreadForParticipants_data();
    void testThreadForParticipants();
    void testQueryEvents();
    void testQueryThreads();

private:
    History::Manager *mManager;
};

void ManagerTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::MatchFlags>();
    qRegisterMetaType<History::Threads>();
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
                                                    << (QStringList() << "+1234567890")
                                                    << History::MatchFlags(History::MatchPhoneNumber)
                                                    << (QStringList() << "4567890");
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
    QCOMPARE(thread.participants(), participants);

    // now try to get the thread again to see if it is returned correctly
    History::Thread sameThread = mManager->threadForParticipants(accountId, type, participantsToMatch, matchFlags, false);
    QVERIFY(sameThread == thread);
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

QTEST_MAIN(ManagerTest)
#include "ManagerTest.moc"

