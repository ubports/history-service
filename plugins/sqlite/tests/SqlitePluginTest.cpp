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
#include "sqlitehistoryplugin.h"
#include "sqlitedatabase.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::MatchFlags)

class SqlitePluginTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCreateThread_data();
    void testCreateThread();
    void testThreadForParticipants_data();
    void testThreadForParticipants();
    void testEmptyThreadForParticipants();
    void testGetSingleThread();
    void testRemoveThread();
    void testBatchOperation();
    void testRollback();

private:
    SQLiteHistoryPlugin *mPlugin;
};

void SqlitePluginTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::MatchFlags>();

    qputenv("HISTORY_SQLITE_DBPATH", ":memory:");
    mPlugin = new SQLiteHistoryPlugin(this);
}

void SqlitePluginTest::testCreateThread_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<History::EventType>("eventType");
    QTest::addColumn<QStringList>("participants");

    QTest::newRow("voice thread with one participant") << "oneAccount" << History::EventTypeVoice << (QStringList() << "oneParticipant");
    QTest::newRow("text thread with multiple participants") << "oneAccount" << History::EventTypeText << (QStringList() << "first" << "second");
}

void SqlitePluginTest::testCreateThread()
{
    QFETCH(QString, accountId);
    QFETCH(History::EventType, eventType);
    QFETCH(QStringList, participants);

    SQLiteDatabase::instance()->reopen();

    QVariantMap thread = mPlugin->createThreadForParticipants(accountId, eventType, participants);

    // check that the variant map is properly filled
    QCOMPARE(thread[History::FieldAccountId].toString(), accountId);
    QCOMPARE(thread[History::FieldType].toInt(), (int) eventType);
    QCOMPARE(thread[History::FieldParticipants].toStringList(), participants);
    QVERIFY(!thread[History::FieldThreadId].toString().isEmpty());

    // now check that the thread is properly saved in the database
    QSqlQuery query(SQLiteDatabase::instance()->database());
    query.prepare("SELECT * FROM threads WHERE accountId=:accountId AND type=:type");
    query.bindValue(":accountId", accountId);
    query.bindValue(":type", (int) eventType);
    QVERIFY(query.exec());

    int count = 0;
    while (query.next()) {
        count++;
        QCOMPARE(query.value("accountId"), thread[History::FieldAccountId]);
        QCOMPARE(query.value("threadId"), thread[History::FieldThreadId]);
        QCOMPARE(query.value("type"), thread[History::FieldType]);
        QCOMPARE(query.value("count").toInt(), 0);
        QCOMPARE(query.value("unreadCount").toInt(), 0);
    }
    QCOMPARE(count, 1);

    // and make sure all the participants are saved correctly
    query.prepare("SELECT * FROM thread_participants");
    QVERIFY(query.exec());

    count = 0;
    while (query.next()) {
        count++;
        QCOMPARE(query.value("accountId"), thread[History::FieldAccountId]);
        QCOMPARE(query.value("threadId"), thread[History::FieldThreadId]);
        QCOMPARE(query.value("type"), thread[History::FieldType]);
        QVERIFY(participants.contains(query.value("participantId").toString()));
    }
    QCOMPARE(count, participants.count());
}

void SqlitePluginTest::testThreadForParticipants_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<History::EventType>("eventType");
    QTest::addColumn<QStringList>("participants");
    QTest::addColumn<History::MatchFlags>("matchFlags");
    QTest::addColumn<QStringList>("participantsToMatch");

    QTest::newRow("voice thread with one participant") << "oneAccount"
                                                       << History::EventTypeVoice
                                                       << (QStringList() << "oneParticipant")
                                                       << History::MatchFlags(History::MatchCaseSensitive)
                                                       << (QStringList() << "oneParticipant");
    QTest::newRow("text thread with multiple participants") << "oneAccount"
                                                            << History::EventTypeText
                                                            << (QStringList() << "first" << "second" << "third")
                                                            << History::MatchFlags(History::MatchCaseSensitive)
                                                            << (QStringList() << "second" << "first" << "third");
    QTest::newRow("phone number match with one participant") << "thePhoneAccount"
                                                             << History::EventTypeVoice
                                                             << (QStringList() << "+1234567890")
                                                             << History::MatchFlags(History::MatchPhoneNumber)
                                                             << (QStringList() << "234567890");
    QTest::newRow("phone number match with multiple participants") << "phoneAccount"
                                                                   << History::EventTypeText
                                                                   << (QStringList() << "1234567" << "+19999999999")
                                                                   << History::MatchFlags(History::MatchPhoneNumber)
                                                                   << (QStringList() << "+55411234567" << "99999999");
}

void SqlitePluginTest::testThreadForParticipants()
{
    QFETCH(QString, accountId);
    QFETCH(History::EventType, eventType);
    QFETCH(QStringList, participants);
    QFETCH(History::MatchFlags, matchFlags);
    QFETCH(QStringList, participantsToMatch);

    SQLiteDatabase::instance()->reopen();

    QVariantMap thread = mPlugin->createThreadForParticipants(accountId, eventType, participants);
    // there is no need to check the results of createThreadForParticipants, they are tested in another function
    // just check if the resulting thread is not empty
    QVERIFY(!thread.isEmpty());

    // now try to fetch the thread for the given participants
    QVariantMap retrievedThread = mPlugin->threadForParticipants(accountId, eventType, participantsToMatch, matchFlags);
    QVERIFY(!retrievedThread.isEmpty());
    QCOMPARE(retrievedThread[History::FieldAccountId], thread[History::FieldAccountId]);
    QCOMPARE(retrievedThread[History::FieldThreadId], thread[History::FieldThreadId]);
    QCOMPARE(retrievedThread[History::FieldType], thread[History::FieldType]);
    QCOMPARE(retrievedThread[History::FieldCount], thread[History::FieldCount]);
    QCOMPARE(retrievedThread[History::FieldUnreadCount], thread[History::FieldUnreadCount]);
    QCOMPARE(retrievedThread[History::FieldParticipants], thread[History::FieldParticipants]);
}

void SqlitePluginTest::testEmptyThreadForParticipants()
{
    QVariantMap thread = mPlugin->threadForParticipants("randomAccount", History::EventTypeText, QStringList());
    QVERIFY(thread.isEmpty());
}

void SqlitePluginTest::testGetSingleThread()
{
    // reset the database
    SQLiteDatabase::instance()->reopen();

    // create the thread that we will retrieve later
    QVariantMap thread = mPlugin->createThreadForParticipants("theAccountId", History::EventTypeText, QStringList() << "theParticipant");
    QVERIFY(!thread.isEmpty());

    // now create some other threads just to make sure the correct one is retrieved
    mPlugin->createThreadForParticipants("theAccountId", History::EventTypeText, QStringList() << "otherParticipant");
    mPlugin->createThreadForParticipants("theAccountId", History::EventTypeVoice, QStringList() << "theParticipant");
    mPlugin->createThreadForParticipants("otherAccount", History::EventTypeText, QStringList() << "theParticipant");

    // and now retrieve the thread
    QVariantMap retrievedThread = mPlugin->getSingleThread(History::EventTypeText, "theAccountId", thread[History::FieldThreadId].toString());
    QCOMPARE(retrievedThread[History::FieldAccountId], thread[History::FieldAccountId]);
    QCOMPARE(retrievedThread[History::FieldThreadId], thread[History::FieldThreadId]);
    QCOMPARE(retrievedThread[History::FieldType], thread[History::FieldType]);
    QCOMPARE(retrievedThread[History::FieldCount], thread[History::FieldCount]);
    QCOMPARE(retrievedThread[History::FieldUnreadCount], thread[History::FieldUnreadCount]);
    QCOMPARE(retrievedThread[History::FieldParticipants], thread[History::FieldParticipants]);

    // FIXME: check that the last event data is also present
}

void SqlitePluginTest::testRemoveThread()
{
    // reset the database
    SQLiteDatabase::instance()->reopen();

    QVariantMap thread = mPlugin->createThreadForParticipants("oneAccountId", History::EventTypeText, QStringList() << "oneParticipant");
    QVERIFY(!thread.isEmpty());
    QVERIFY(mPlugin->removeThread(thread));

    // now check that the thread was really removed
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT count(*) FROM threads"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);

    // and check that the participants were also removed
    QVERIFY(query.exec("SELECT count(*) FROM thread_participants"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);
}

void SqlitePluginTest::testBatchOperation()
{
    // clear the database
    SQLiteDatabase::instance()->reopen();

    QVERIFY(mPlugin->beginBatchOperation());
    mPlugin->createThreadForParticipants("accountOne", History::EventTypeText, QStringList() << "participantOne");
    mPlugin->createThreadForParticipants("accountTwo", History::EventTypeText, QStringList() << "participantTwo");
    mPlugin->createThreadForParticipants("accountThree", History::EventTypeText, QStringList() << "participantThree");
    QVERIFY(mPlugin->endBatchOperation());

    // check that the data was actually written
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT count(*) FROM threads"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 3);
}

void SqlitePluginTest::testRollback()
{
    // clear the database
    SQLiteDatabase::instance()->reopen();

    QVERIFY(mPlugin->beginBatchOperation());
    mPlugin->createThreadForParticipants("accountOne", History::EventTypeText, QStringList() << "participantOne");
    mPlugin->createThreadForParticipants("accountTwo", History::EventTypeText, QStringList() << "participantTwo");
    mPlugin->createThreadForParticipants("accountThree", History::EventTypeText, QStringList() << "participantThree");
    QVERIFY(mPlugin->rollbackBatchOperation());

    // check that the steps were reverted
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT count(*) FROM threads"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);
}

QTEST_MAIN(SqlitePluginTest)
#include "SqlitePluginTest.moc"

