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
#include "sqlitehistorythreadview.h"
#include "sqlitehistoryeventview.h"
#include "textevent.h"
#include "texteventattachment.h"
#include "voiceevent.h"

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
    void testQueryThreads();
    void testQueryEvents();
    void testWriteTextEvent_data();
    void testWriteTextEvent();
    void testWriteVoiceEvent_data();
    void testWriteVoiceEvent();

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

void SqlitePluginTest::testQueryThreads()
{
    // just make sure the returned view is of the correct type. The views are going to be tested in their own tests
    History::PluginThreadView *view = mPlugin->queryThreads(History::EventTypeVoice);
    QVERIFY(view);
    QVERIFY(dynamic_cast<SQLiteHistoryThreadView*>(view));
    view->deleteLater();
}

void SqlitePluginTest::testQueryEvents()
{
    // just make sure the returned view is of the correct type. The views are going to be tested in their own tests
    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeText);
    QVERIFY(view);
    QVERIFY(dynamic_cast<SQLiteHistoryEventView*>(view));
    view->deleteLater();
}

void SqlitePluginTest::testWriteTextEvent_data()
{
    QTest::addColumn<QVariantMap>("event");

    // for test purposes, the threadId == senderId to make it easier
    QTest::newRow("new text event with pending flag") << History::TextEvent("oneAccountId", "theSender", "oneEventId",
                                                                            "theSender", QDateTime::currentDateTime(), true,
                                                                            "Hello World!", History::MessageTypeText,
                                                                            History::MessageFlagPending).properties();
    QTest::newRow("text event with valid read timestamp") << History::TextEvent("otherAccountId", "otherSender", "otherEventId",
                                                                                "otherSender", QDateTime::currentDateTime(), false,
                                                                                "Hi Again!", History::MessageTypeText,
                                                                                History::MessageFlagDelivered,
                                                                                QDateTime::currentDateTime()).properties();
    History::TextEventAttachments attachments;
    attachments << History::TextEventAttachment("mmsAccountId", "mmsSender", "mmsEventId", "mmsAttachment1",
                                                "text/plain", "/the/file/path", History::AttachmentDownloaded);
    QTest::newRow("text event with attachments") << History::TextEvent("mmsAccountId", "mmsSender", "mmsEventId", "mmsSender",
                                                                       QDateTime::currentDateTime(), false, "Hello with attachments",
                                                                       History::MessageTypeMultiParty, History::MessageFlagDelivered,
                                                                       QDateTime::currentDateTime(), "The Subject", attachments).properties();
}

void SqlitePluginTest::testWriteTextEvent()
{
    QFETCH(QVariantMap, event);
    // clear the database
    SQLiteDatabase::instance()->reopen();

    // create the thread
    QVariantMap thread = mPlugin->createThreadForParticipants(event[History::FieldAccountId].toString(),
                                                              History::EventTypeText,
                                                              QStringList() << event[History::FieldSenderId].toString());
    QVERIFY(!thread.isEmpty());


    // write the text event
    History::EventWriteResult result = mPlugin->writeTextEvent(event);
    QCOMPARE(result, History::EventWriteCreated);

    // check that the event is properly written to the database
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT * FROM text_events"));
    int count = 0;
    while(query.next()) {
        count++;
        QCOMPARE(query.value("accountId"), event[History::FieldAccountId]);
        QCOMPARE(query.value("threadId"), event[History::FieldThreadId]);
        QCOMPARE(query.value("eventId"), event[History::FieldEventId]);
        QCOMPARE(query.value("senderId"), event[History::FieldSenderId]);
        QCOMPARE(query.value("timestamp"), event[History::FieldTimestamp]);
        QCOMPARE(query.value("newEvent"), event[History::FieldNewEvent]);
        QCOMPARE(query.value("message"), event[History::FieldMessage]);
        QCOMPARE(query.value("messageType"), event[History::FieldMessageType]);
        QCOMPARE(query.value("messageFlags"), event[History::FieldMessageFlags]);
        QCOMPARE(query.value("readTimestamp"), event[History::FieldReadTimestamp]);
        QCOMPARE(query.value("subject"), event[History::FieldSubject]);
    }

    // check that only one event got written
    QCOMPARE(count, 1);

    // check that the attachments got saved, if any
    if (event[History::FieldMessageType].toInt() == History::MessageTypeMultiParty) {
        QVariantMap attachment = event[History::FieldAttachments].value<QList<QVariantMap> >()[0];
        QVERIFY(query.exec("SELECT * FROM text_event_attachments"));
        int count = 0;
        while(query.next()) {
            count++;
            QCOMPARE(query.value("accountId"), attachment[History::FieldAccountId]);
            QCOMPARE(query.value("threadId"), attachment[History::FieldThreadId]);
            QCOMPARE(query.value("eventId"), attachment[History::FieldEventId]);
            QCOMPARE(query.value("attachmentId"), attachment[History::FieldAttachmentId]);
            QCOMPARE(query.value("contentType"), attachment[History::FieldContentType]);
            QCOMPARE(query.value("filePath"), attachment[History::FieldFilePath]);
            QCOMPARE(query.value("status"), attachment[History::FieldStatus]);
        }
        QCOMPARE(count, 1);
    }

    // and check that the thread's last item got updated
    thread = mPlugin->getSingleThread(History::EventTypeText, thread[History::FieldAccountId].toString(), thread[History::FieldThreadId].toString());
    QCOMPARE(thread[History::FieldAccountId], event[History::FieldAccountId]);
    QCOMPARE(thread[History::FieldThreadId], event[History::FieldThreadId]);
    QCOMPARE(thread[History::FieldEventId], event[History::FieldEventId]);
    QCOMPARE(thread[History::FieldSenderId], event[History::FieldSenderId]);
    QCOMPARE(thread[History::FieldTimestamp], event[History::FieldTimestamp]);
    QCOMPARE(thread[History::FieldNewEvent], event[History::FieldNewEvent]);
    QCOMPARE(thread[History::FieldMessage], event[History::FieldMessage]);
    QCOMPARE(thread[History::FieldMessageType], event[History::FieldMessageType]);
    QCOMPARE(thread[History::FieldMessageFlags], event[History::FieldMessageFlags]);
    QCOMPARE(thread[History::FieldReadTimestamp], event[History::FieldReadTimestamp]);
}

void SqlitePluginTest::testWriteVoiceEvent_data()
{
    QTest::addColumn<QVariantMap>("event");

    // for test purposes, the threadId == senderId to make it easier
    QTest::newRow("missed call") << History::VoiceEvent("theAccountId", "theSenderId", "theEventId", "theSenderId",
                                                        QDateTime::currentDateTime(), true, true).properties();
    QTest::newRow("incoming call") << History::VoiceEvent("otherAccountId", "otherSenderId", "otherEventId", "otherSenderId",
                                                          QDateTime::currentDateTime(), false, false, QTime(0,10,30)).properties();
}

void SqlitePluginTest::testWriteVoiceEvent()
{
    QFETCH(QVariantMap, event);
    // clear the database
    SQLiteDatabase::instance()->reopen();

    // create the thread
    QVariantMap thread = mPlugin->createThreadForParticipants(event[History::FieldAccountId].toString(),
                                                              History::EventTypeVoice,
                                                              QStringList() << event[History::FieldSenderId].toString());
    QVERIFY(!thread.isEmpty());


    // write the voice event
    History::EventWriteResult result = mPlugin->writeVoiceEvent(event);
    QCOMPARE(result, History::EventWriteCreated);

    // check that the event is properly written to the database
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT * FROM voice_events"));
    int count = 0;
    while(query.next()) {
        count++;
        QCOMPARE(query.value("accountId"), event[History::FieldAccountId]);
        QCOMPARE(query.value("threadId"), event[History::FieldThreadId]);
        QCOMPARE(query.value("eventId"), event[History::FieldEventId]);
        QCOMPARE(query.value("senderId"), event[History::FieldSenderId]);
        QCOMPARE(query.value("timestamp"), event[History::FieldTimestamp]);
        QCOMPARE(query.value("newEvent"), event[History::FieldNewEvent]);
        QCOMPARE(query.value("missed"), event[History::FieldMissed]);
        QCOMPARE(query.value("duration"), event[History::FieldDuration]);
    }

    // check that only one event got written
    QCOMPARE(count, 1);

    // and check that the thread's last item got updated
    thread = mPlugin->getSingleThread(History::EventTypeVoice, thread[History::FieldAccountId].toString(), thread[History::FieldThreadId].toString());
    QCOMPARE(thread[History::FieldAccountId], event[History::FieldAccountId]);
    QCOMPARE(thread[History::FieldThreadId], event[History::FieldThreadId]);
    QCOMPARE(thread[History::FieldEventId], event[History::FieldEventId]);
    QCOMPARE(thread[History::FieldSenderId], event[History::FieldSenderId]);
    QCOMPARE(thread[History::FieldTimestamp], event[History::FieldTimestamp]);
    QCOMPARE(thread[History::FieldNewEvent], event[History::FieldNewEvent]);
    QCOMPARE(thread[History::FieldMissed], event[History::FieldMissed]);
    QCOMPARE(thread[History::FieldDuration], event[History::FieldDuration]);
}

QTEST_MAIN(SqlitePluginTest)
#include "SqlitePluginTest.moc"

