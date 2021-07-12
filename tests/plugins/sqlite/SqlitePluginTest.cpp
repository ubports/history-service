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
#include "intersectionfilter.h"
#include "unionfilter.h"

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
    void testModifyTextEvent();
    void testRemoveTextEvent();
    void testWriteVoiceEvent_data();
    void testWriteVoiceEvent();
    void testModifyVoiceEvent();
    void testRemoveVoiceEvent();
    void testEventsForThread();
    void testGetSingleEvent_data();
    void testGetSingleEvent();
    void testFilterToString_data();
    void testFilterToString();
    void testEscapeFilterValue_data();
    void testEscapeFilterValue();

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
    QCOMPARE(History::Participants::fromVariantList(thread[History::FieldParticipants].toList()).identifiers(), participants);
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
                                                             << (QStringList() << "+12345678901")
                                                             << History::MatchFlags(History::MatchPhoneNumber)
                                                             << (QStringList() << "2345678901");
    QTest::newRow("phone number match with multiple participants") << "phoneAccount"
                                                                   << History::EventTypeText
                                                                   << (QStringList() << "12345678" << "+19999999999")
                                                                   << History::MatchFlags(History::MatchPhoneNumber)
                                                                   << (QStringList() << "+554112345678" << "9999999");
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
    QSqlQuery query(SQLiteDatabase::instance()->database());

    QVERIFY(mPlugin->beginBatchOperation());
    QVERIFY(query.exec("UPDATE schema_version SET version=123"));
    QVERIFY(mPlugin->endBatchOperation());

    // check that the data was actually written
    QVERIFY(query.exec("SELECT version FROM schema_version"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 123);
}

void SqlitePluginTest::testRollback()
{
    // clear the database
    SQLiteDatabase::instance()->reopen();
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT version FROM schema_version"));
    QVERIFY(query.next());
    int version = query.value(0).toInt();

    QVERIFY(mPlugin->beginBatchOperation());
    QVERIFY(query.exec("UPDATE schema_version SET version=255"));
    QVERIFY(mPlugin->rollbackBatchOperation());

    // check that the steps were reverted
    QVERIFY(query.exec("SELECT version FROM schema_version"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), version);
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
                                                                            "theSender", QDateTime::currentDateTime(), QDateTime::currentDateTime(), true,
                                                                            "Hello World!", History::MessageTypeText,
                                                                            History::MessageStatusPending).properties();
    QTest::newRow("text event with valid read timestamp") << History::TextEvent("otherAccountId", "otherSender", "otherEventId",
                                                                                "otherSender", QDateTime::currentDateTime(), QDateTime::currentDateTime(), false,
                                                                                "Hi Again!", History::MessageTypeText,
                                                                                History::MessageStatusDelivered,
                                                                                QDateTime::currentDateTime()).properties();
    QTest::newRow("text event with no sent time") << History::TextEvent("noSentAccountId", "noSentSender", "noSentEventId",
                                                                                "noSentSender", QDateTime::currentDateTime(), false,
                                                                                "Hi Again!", History::MessageTypeText,
                                                                                History::MessageStatusDelivered,
                                                                                QDateTime::currentDateTime()).properties();
    History::TextEventAttachments attachments;
    attachments << History::TextEventAttachment("mmsAccountId", "mmsSender", "mmsEventId", "mmsAttachment1",
                                                "text/plain", "/the/file/path", History::AttachmentDownloaded);
    QTest::newRow("text event with attachments") << History::TextEvent("mmsAccountId", "mmsSender", "mmsEventId", "mmsSender",
                                                                       QDateTime::currentDateTime(), QDateTime::currentDateTime(), false, "Hello with attachments",
                                                                       History::MessageTypeMultiPart, History::MessageStatusDelivered,
                                                                       QDateTime::currentDateTime(), "The Subject", History::InformationTypeNone, attachments).properties();
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
        QCOMPARE(mPlugin->toLocalTimeString(query.value("timestamp").toDateTime()),
                 event[History::FieldTimestamp].toString());
        QCOMPARE(mPlugin->toLocalTimeString(query.value("sentTime").toDateTime()),
                 event[History::FieldSentTime].toString());
        QCOMPARE(query.value("newEvent"), event[History::FieldNewEvent]);
        QCOMPARE(query.value("message"), event[History::FieldMessage]);
        QCOMPARE(query.value("messageType"), event[History::FieldMessageType]);
        QCOMPARE(query.value("messageStatus"), event[History::FieldMessageStatus]);
        QCOMPARE(mPlugin->toLocalTimeString(query.value("readTimestamp").toDateTime()),
                 event[History::FieldReadTimestamp].toString());
        QCOMPARE(query.value("subject"), event[History::FieldSubject]);
    }

    // check that only one event got written
    QCOMPARE(count, 1);

    // check that the attachments got saved, if any
    if (event[History::FieldMessageType].toInt() == History::MessageTypeMultiPart) {
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
    QCOMPARE(thread[History::FieldMessageStatus], event[History::FieldMessageStatus]);
    QCOMPARE(thread[History::FieldReadTimestamp], event[History::FieldReadTimestamp]);
}

void SqlitePluginTest::testModifyTextEvent()
{
    // clear the database
    SQLiteDatabase::instance()->reopen();

    QVariantMap thread = mPlugin->createThreadForParticipants("theAccountId", History::EventTypeText, QStringList() << "theParticipant");
    QVERIFY(!thread.isEmpty());
    History::TextEventAttachment attachment(thread[History::FieldAccountId].toString(), thread[History::FieldThreadId].toString(),
                                            thread[History::FieldEventId].toString(), "theAttachmentId", "text/plain", "/file/path");
    History::TextEvent textEvent(thread[History::FieldAccountId].toString(), thread[History::FieldThreadId].toString(), "theEventId",
                                 "theParticipant", QDateTime::currentDateTime(), QDateTime::currentDateTime(), true, "Hi there!", History::MessageTypeMultiPart,
                                 History::MessageStatusPending, QDateTime::currentDateTime(), "theSubject", History::InformationTypeNone,
                                 History::TextEventAttachments() << attachment);
    QCOMPARE(mPlugin->writeTextEvent(textEvent.properties()), History::EventWriteCreated);

    // now modify the fields that can be modified in the class
    textEvent.setNewEvent(false);
    textEvent.setMessageStatus(History::MessageStatusDelivered);
    textEvent.setReadTimestamp(QDateTime::currentDateTime());
    QCOMPARE(mPlugin->writeTextEvent(textEvent.properties()), History::EventWriteModified);

    // and check that the data is actually up-to-date in the database
    QVariantMap event = textEvent.properties();
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT * FROM text_events"));
    int count = 0;
    while(query.next()) {
        count++;
        QCOMPARE(query.value("accountId"), event[History::FieldAccountId]);
        QCOMPARE(query.value("threadId"), event[History::FieldThreadId]);
        QCOMPARE(query.value("eventId"), event[History::FieldEventId]);
        QCOMPARE(query.value("senderId"), event[History::FieldSenderId]);
        QCOMPARE(mPlugin->toLocalTimeString(query.value("timestamp").toDateTime()),
                 event[History::FieldTimestamp].toString());
        QCOMPARE(mPlugin->toLocalTimeString(query.value("sentTime").toDateTime()),
                 event[History::FieldSentTime].toString());
        QCOMPARE(query.value("newEvent"), event[History::FieldNewEvent]);
        QCOMPARE(query.value("message"), event[History::FieldMessage]);
        QCOMPARE(query.value("messageType"), event[History::FieldMessageType]);
        QCOMPARE(query.value("messageStatus"), event[History::FieldMessageStatus]);
        QCOMPARE(mPlugin->toLocalTimeString(query.value("readTimestamp").toDateTime()),
                 event[History::FieldReadTimestamp].toString());
        QCOMPARE(query.value("subject"), event[History::FieldSubject]);
    }

    // check that only one event got written
    QCOMPARE(count, 1);
}

void SqlitePluginTest::testRemoveTextEvent()
{
    // clear the database
    SQLiteDatabase::instance()->reopen();

    QVariantMap thread = mPlugin->createThreadForParticipants("theAccountId", History::EventTypeText, QStringList() << "theParticipant");
    QVERIFY(!thread.isEmpty());

    History::TextEvent textEvent(thread[History::FieldAccountId].toString(),
                                 thread[History::FieldThreadId].toString(), "theSenderId",
                                 "theEventId", QDateTime::currentDateTime(),  QDateTime::currentDateTime(), true,
                                 "Hello World!", History::MessageTypeText,
                                 History::MessageStatusPending);
    QCOMPARE(mPlugin->writeTextEvent(textEvent.properties()), History::EventWriteCreated);

    // now remove the item and check that it is really removed from the database
    QVERIFY(mPlugin->removeTextEvent(textEvent.properties()));

    // check that the event was removed from the database
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT count(*) FROM text_events"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);
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
        QCOMPARE(mPlugin->toLocalTimeString(query.value("timestamp").toDateTime()),
                 event[History::FieldTimestamp].toString());
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

void SqlitePluginTest::testModifyVoiceEvent()
{
    // clear the database
    SQLiteDatabase::instance()->reopen();

    QVariantMap thread = mPlugin->createThreadForParticipants("theAccountId", History::EventTypeVoice, QStringList() << "theParticipant");
    QVERIFY(!thread.isEmpty());
    History::VoiceEvent voiceEvent(thread[History::FieldAccountId].toString(), thread[History::FieldThreadId].toString(), "theEventId",
                                   "theParticipant", QDateTime::currentDateTime(), true, true, QTime(0, 1, 2));
    QCOMPARE(mPlugin->writeVoiceEvent(voiceEvent.properties()), History::EventWriteCreated);

    // now modify the fields that can be modified in the class
    voiceEvent.setNewEvent(false);
    QCOMPARE(mPlugin->writeVoiceEvent(voiceEvent.properties()), History::EventWriteModified);

    // and check that the data is actually up-to-date in the database
    QVariantMap event = voiceEvent.properties();
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT * FROM voice_events"));
    int count = 0;
    while(query.next()) {
        count++;
        QCOMPARE(query.value("accountId"), event[History::FieldAccountId]);
        QCOMPARE(query.value("threadId"), event[History::FieldThreadId]);
        QCOMPARE(query.value("eventId"), event[History::FieldEventId]);
        QCOMPARE(query.value("senderId"), event[History::FieldSenderId]);
        QCOMPARE(mPlugin->toLocalTimeString(query.value("timestamp").toDateTime()),
                 event[History::FieldTimestamp].toString());
        QCOMPARE(query.value("newEvent"), event[History::FieldNewEvent]);
        QCOMPARE(query.value("missed"), event[History::FieldMissed]);
        QCOMPARE(query.value("duration"), event[History::FieldDuration]);
    }

    // check that only one event got written
    QCOMPARE(count, 1);
}

void SqlitePluginTest::testRemoveVoiceEvent()
{
    // clear the database
    SQLiteDatabase::instance()->reopen();

    QVariantMap thread = mPlugin->createThreadForParticipants("theAccountId", History::EventTypeVoice, QStringList() << "theParticipant");
    QVERIFY(!thread.isEmpty());

    History::VoiceEvent voiceEvent(thread[History::FieldAccountId].toString(),
                                   thread[History::FieldThreadId].toString(), "theSenderId",
                                   "theEventId", QDateTime::currentDateTime(), true,
                                   true, QTime(0, 5, 10));
    QCOMPARE(mPlugin->writeVoiceEvent(voiceEvent.properties()), History::EventWriteCreated);

    // now remove the item and check that it is really removed from the database
    QVERIFY(mPlugin->removeVoiceEvent(voiceEvent.properties()));

    // check that the event was removed from the database
    QSqlQuery query(SQLiteDatabase::instance()->database());
    QVERIFY(query.exec("SELECT count(*) FROM voice_events"));
    QVERIFY(query.next());
    QCOMPARE(query.value(0).toInt(), 0);
}

void SqlitePluginTest::testEventsForThread()
{
    // clear the database
    SQLiteDatabase::instance()->reopen();

    // test text events
    QVariantMap textThread = mPlugin->createThreadForParticipants("textAccountId", History::EventTypeText, QStringList() << "textParticipant");
    QVERIFY(!textThread.isEmpty());

    // now insert 50 events
    for (int i = 0; i < 50; ++i) {
        History::TextEventAttachment attachment(textThread[History::FieldAccountId].toString(), textThread[History::FieldThreadId].toString(),
                                                QString("textEventId%1").arg(QString::number(i)), QString("attachment%1").arg(i),
                                                "text/plain", "/some/file/path");
        History::TextEvent textEvent(textThread[History::FieldAccountId].toString(), textThread[History::FieldThreadId].toString(),
                                     QString("textEventId%1").arg(QString::number(i)), "textParticipant",
                                     QDateTime::currentDateTime(), QDateTime::currentDateTime().addSecs(-10), true, "Hello World!", History::MessageTypeMultiPart,
                                     History::MessageStatusPending, QDateTime::currentDateTime(),
                                     "theSubject", History::InformationTypeNone, History::TextEventAttachments() << attachment);
        QCOMPARE(mPlugin->writeTextEvent(textEvent.properties()), History::EventWriteCreated);
    }

    QList<QVariantMap> textEvents = mPlugin->eventsForThread(textThread);
    QCOMPARE(textEvents.count(), 50);
    Q_FOREACH(const QVariantMap &textEvent, textEvents) {
        QCOMPARE(textEvent[History::FieldAccountId], textThread[History::FieldAccountId]);
        QCOMPARE(textEvent[History::FieldThreadId], textThread[History::FieldThreadId]);
        QCOMPARE(textEvent[History::FieldType], textThread[History::FieldType]);
    }

    // test voice events
    QVariantMap voiceThread = mPlugin->createThreadForParticipants("voiceAccountId", History::EventTypeVoice, QStringList() << "voiceParticipant");
    QVERIFY(!voiceThread.isEmpty());

    // now insert 50 events
    for (int i = 0; i < 50; ++i) {
        History::VoiceEvent voiceEvent(voiceThread[History::FieldAccountId].toString(), voiceThread[History::FieldThreadId].toString(),
                                       QString("voiceEventId%1").arg(QString::number(i)), "voiceParticipant",
                                       QDateTime::currentDateTime(), true, false, QTime(0, i, i));
        QCOMPARE(mPlugin->writeVoiceEvent(voiceEvent.properties()), History::EventWriteCreated);
    }

    QList<QVariantMap> voiceEvents = mPlugin->eventsForThread(voiceThread);
    QCOMPARE(voiceEvents.count(), 50);
    Q_FOREACH(const QVariantMap &voiceEvent, voiceEvents) {
        QCOMPARE(voiceEvent[History::FieldAccountId], voiceThread[History::FieldAccountId]);
        QCOMPARE(voiceEvent[History::FieldThreadId], voiceThread[History::FieldThreadId]);
        QCOMPARE(voiceEvent[History::FieldType], voiceThread[History::FieldType]);
    }
}

void SqlitePluginTest::testGetSingleEvent_data()
{
    QTest::addColumn<QVariantMap>("event");

    // for test purposes, the threadId == senderId to make it easier
    QTest::newRow("new text event with pending flag") << History::TextEvent("oneAccountId", "theSender", "oneEventId",
                                                                            "theSender", QDateTime::currentDateTime(), QDateTime::currentDateTime(), true,
                                                                            "Hello World!", History::MessageTypeText,
                                                                            History::MessageStatusPending).properties();
    QTest::newRow("text event with valid read timestamp") << History::TextEvent("otherAccountId", "otherSender", "otherEventId",
                                                                                "otherSender", QDateTime::currentDateTime(), QDateTime::currentDateTime(), false,
                                                                                "Hi Again!", History::MessageTypeText,
                                                                                History::MessageStatusDelivered,
                                                                                QDateTime::currentDateTime()).properties();
    QTest::newRow("text event with no sent time") << History::TextEvent("noSentAccountId", "noSentSender", "noSentEventId",
                                                                                "noSentSender", QDateTime::currentDateTime(), false,
                                                                                "Hi Again!", History::MessageTypeText,
                                                                                History::MessageStatusDelivered,
                                                                                QDateTime::currentDateTime()).properties();
    History::TextEventAttachments attachments;
    attachments << History::TextEventAttachment("mmsAccountId", "mmsSender", "mmsEventId", "mmsAttachment1",
                                                "text/plain", "/the/file/path", History::AttachmentDownloaded);
    QTest::newRow("text event with attachments") << History::TextEvent("mmsAccountId", "mmsSender", "mmsEventId", "mmsSender",
                                                                       QDateTime::currentDateTime(), QDateTime::currentDateTime(), false, "Hello with attachments",
                                                                       History::MessageTypeMultiPart, History::MessageStatusDelivered,
                                                                       QDateTime::currentDateTime(), "The Subject", History::InformationTypeNone, attachments).properties();
    QTest::newRow("missed call") << History::VoiceEvent("theAccountId", "theSenderId", "theEventId", "theSenderId",
                                                        QDateTime::currentDateTime(), true, true).properties();
    QTest::newRow("incoming call") << History::VoiceEvent("otherAccountId", "otherSenderId", "otherEventId", "otherSenderId",
                                                          QDateTime::currentDateTime(), false, false, QTime(0,10,30)).properties();
}

void SqlitePluginTest::testGetSingleEvent()
{
    QFETCH(QVariantMap, event);
    // clear the database
    SQLiteDatabase::instance()->reopen();

    History::EventType type = (History::EventType) event[History::FieldType].toInt();

    // create the thread
    QVariantMap thread = mPlugin->createThreadForParticipants(event[History::FieldAccountId].toString(),
                                                              type,
                                                              QStringList() << event[History::FieldSenderId].toString());
    QVERIFY(!thread.isEmpty());


    // write the event
    switch (type) {
    case History::EventTypeText:
        QCOMPARE(mPlugin->writeTextEvent(event), History::EventWriteCreated);
        break;
    case History::EventTypeVoice:
        QCOMPARE(mPlugin->writeVoiceEvent(event), History::EventWriteCreated);
        break;
    case History::EventTypeNull:
        break;
    }

    QVariantMap retrievedEvent = mPlugin->getSingleEvent(type, event[History::FieldAccountId].toString(),
                                                         event[History::FieldThreadId].toString(),
                                                         event[History::FieldEventId].toString());
    QCOMPARE(retrievedEvent[History::FieldAccountId], event[History::FieldAccountId]);
    QCOMPARE(retrievedEvent[History::FieldThreadId], event[History::FieldThreadId]);
    QCOMPARE(retrievedEvent[History::FieldEventId], event[History::FieldEventId]);
    QCOMPARE(retrievedEvent[History::FieldSenderId], event[History::FieldSenderId]);
    QCOMPARE(retrievedEvent[History::FieldTimestamp], event[History::FieldTimestamp]);
    QCOMPARE(retrievedEvent[History::FieldNewEvent], event[History::FieldNewEvent]);

    switch (type) {
    case History::EventTypeText:
        QCOMPARE(retrievedEvent[History::FieldMessage], event[History::FieldMessage]);
        QCOMPARE(retrievedEvent[History::FieldMessageType], event[History::FieldMessageType]);
        QCOMPARE(retrievedEvent[History::FieldMessageStatus], event[History::FieldMessageStatus]);
        QCOMPARE(retrievedEvent[History::FieldReadTimestamp], event[History::FieldReadTimestamp]);
        QCOMPARE(retrievedEvent[History::FieldSentTime], event[History::FieldSentTime]);
        break;
    case History::EventTypeVoice:
        QCOMPARE(retrievedEvent[History::FieldMissed], event[History::FieldMissed]);
        QCOMPARE(retrievedEvent[History::FieldDuration], event[History::FieldDuration]);
        break;
    case History::EventTypeNull:
        break;
    }
}

void SqlitePluginTest::testFilterToString_data()
{
    QTest::addColumn<QVariantMap>("filterProperties");
    QTest::addColumn<QVariantMap>("filterValues");
    QTest::addColumn<QString>("propertyPrefix");
    QTest::addColumn<QString>("resultString");

    History::Filter filter;
    QVariantMap filterValues;
    filter.setFilterProperty("testProperty");
    filter.setFilterValue("stringValue");
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("simple string filter") << filter.properties() << filterValues << QString() << "testProperty=:filterValue0";

    filter.setFilterValue(12345);
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("simple integer filter") << filter.properties() << filterValues << QString() << "testProperty=:filterValue0";

    filter.setFilterValue(true);
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("simple true boolean filter") << filter.properties() << filterValues << QString() << "testProperty=:filterValue0";

    filter.setFilterValue(false);
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("simple false boolean filter") << filter.properties() << filterValues << QString() << "testProperty=:filterValue0";

    filter.setFilterValue(12345);
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("filter with a prefix") << filter.properties() << filterValues << QString("prefix") << "prefix.testProperty=:filterValue0";

    filter.setFilterValue(12345);
    filter.setMatchFlags(History::MatchLess);
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("filter less") << filter.properties() << filterValues << QString() << "testProperty<:filterValue0";

    filter.setFilterValue(12345);
    filter.setMatchFlags(History::MatchGreater);
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("filter greather") << filter.properties() << filterValues << QString() << "testProperty>:filterValue0";

    filter.setFilterValue(12345);
    filter.setMatchFlags(History::MatchLessOrEquals);
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("filter less or equals") << filter.properties() << filterValues << QString() << "testProperty<=:filterValue0";

    filter.setFilterValue(12345);
    filter.setMatchFlags(History::MatchGreaterOrEquals);
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("filter greater or equals") << filter.properties() << filterValues << QString() << "testProperty>=:filterValue0";

    filter.setFilterValue(12345);
    filter.setMatchFlags(History::MatchNotEquals);
    filterValues[":filterValue0"] = filter.filterValue();
    QTest::newRow("filter not equals") << filter.properties() << filterValues << QString() << "testProperty!=:filterValue0";

    filter.setMatchFlags(History::MatchContains);
    filter.setFilterValue("partialString");
    filterValues.clear();
    QTest::newRow("match contains") << filter.properties() << filterValues << QString() << "testProperty LIKE '\%partialString\%' ESCAPE '\\'";

    filter.setFilterValue("%");
    filterValues.clear();
    QTest::newRow("partial match escaped") << filter.properties() << filterValues << QString() << "testProperty LIKE '\%\\\%\%' ESCAPE '\\'";

    History::IntersectionFilter intersectionFilter;
    filter.setMatchFlags(History::MatchFlags());
    filter.setFilterValue(12345);
    intersectionFilter.append(filter);
    filter.setFilterValue(true);
    intersectionFilter.append(filter);
    filter.setFilterValue("a string");
    intersectionFilter.append(filter);
    filterValues.clear();
    filterValues[":filterValue0"] = 12345;
    filterValues[":filterValue1"] = true;
    filterValues[":filterValue2"] = "a string";
    QTest::newRow("intersection filter") << intersectionFilter.properties() << filterValues << QString() << "( (testProperty=:filterValue0) AND (testProperty=:filterValue1) AND (testProperty=:filterValue2) )";

    History::UnionFilter unionFilter;
    filter.setFilterValue(12345);
    unionFilter.append(filter);
    filter.setFilterValue(true);
    unionFilter.append(filter);
    filter.setFilterValue("a string");
    unionFilter.append(filter);
    filterValues.clear();
    filterValues[":filterValue0"] = 12345;
    filterValues[":filterValue1"] = true;
    filterValues[":filterValue2"] = "a string";
    QTest::newRow("union filter") << unionFilter.properties() << filterValues << QString() << "( (testProperty=:filterValue0) OR (testProperty=:filterValue1) OR (testProperty=:filterValue2) )";
}

void SqlitePluginTest::testFilterToString()
{
    QFETCH(QVariantMap, filterProperties);
    QFETCH(QVariantMap, filterValues);
    QFETCH(QString, propertyPrefix);
    QFETCH(QString, resultString);

    QVariantMap resultValues;
    QString result = mPlugin->filterToString(History::Filter::fromProperties(filterProperties), resultValues, propertyPrefix);
    QCOMPARE(result, resultString);
    QCOMPARE(resultValues, filterValues);

}

void SqlitePluginTest::testEscapeFilterValue_data()
{
    QTest::addColumn<QString>("originalString");
    QTest::addColumn<QString>("escapedString");

    QTest::newRow("backslash") << QString("\\") << QString("\\\\");
    QTest::newRow("single quote") << QString("'") << QString("''");
    QTest::newRow("percent") << QString("%") << QString("\\%");
    QTest::newRow("underscore") << QString("_") << QString("\\_");
    QTest::newRow("string with all of that") << QString("\\0\"'%_bla") << QString("\\\\0\"''\\%\\_bla");
}

void SqlitePluginTest::testEscapeFilterValue()
{
    QFETCH(QString, originalString);
    QFETCH(QString, escapedString);

    QCOMPARE(mPlugin->escapeFilterValue(originalString), escapedString);
}

QTEST_MAIN(SqlitePluginTest)
#include "SqlitePluginTest.moc"
