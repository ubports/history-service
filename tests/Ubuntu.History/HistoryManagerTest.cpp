/*
 * Copyright (C) 2021 UBports Foundation
 *
 * Authors:
 *  Lionel Duboeuf <lduboeuf@ouvaton.org>
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
#include <QJSEngine>
#include "manager.h"
#include "historymanager.h"
#include "voiceevent.h"

class HistoryManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testShouldNotTriggerOperation();
    void testRemoveAll();
private:
    History::Manager *mManager;
};

void HistoryManagerTest::initTestCase()
{
    mManager = History::Manager::instance();
}

void HistoryManagerTest::testShouldNotTriggerOperation()
{
    HistoryManager historyManager;

    QJSEngine myEngine;
    myEngine.globalObject().setProperty("error","");
    myEngine.globalObject().setProperty("deletedEventCount","");
    QJSValue fun = myEngine.evaluate("(function( _deletedEventCount, _error) {"
                                     " error = _error; "
                                     " deletedEventCount = _deletedEventCount;"
                                     "})");
    // no type
    historyManager.removeEvents(History::EventTypeNull, QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz"),fun);
    QCOMPARE(myEngine.globalObject().property("error").toInt(),HistoryManager::OPERATION_INVALID);

    // bad filter
    historyManager.removeEvents(HistoryModel::EventTypeVoice, "", fun);
    QCOMPARE(myEngine.globalObject().property("error").toInt(),HistoryManager::OPERATION_INVALID);

    //start remove no datas
    historyManager.removeEvents(HistoryModel::EventTypeVoice, QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz"),fun);
    QTest::qWait(100);
    QCOMPARE(myEngine.globalObject().property("error").toInt(),HistoryManager::NO_ERROR);
    QCOMPARE(myEngine.globalObject().property("deletedEventCount").toInt(),0);
}

void HistoryManagerTest::testRemoveAll()
{
    HistoryManager historyManager;

    QJSEngine myEngine;
    myEngine.globalObject().setProperty("error","");
    myEngine.globalObject().setProperty("deletedEventCount",-1);
    QJSValue fun = myEngine.evaluate("(function( _deletedEventCount, _error) {"
                                     " error = _error; "
                                     " deletedEventCount = _deletedEventCount;"
                                     "})");

    QString voiceParticipant("voiceParticipant");

    // create a temporary thread to populate the model
    History::Thread voiceThread = mManager->threadForParticipants("voiceAccountId",
                                                                  History::EventTypeVoice,
                                                                  QStringList()<< voiceParticipant,
                                                                  History::MatchCaseSensitive, true);

    History::VoiceEvent voiceEvent(voiceThread.accountId(),
                                   voiceThread.threadId(),
                                   QString("eventId1"),
                                   voiceParticipant,
                                   QDateTime::currentDateTime(),
                                   false,
                                   false,
                                   QTime(1,2,3));
    QVERIFY(mManager->writeEvents(History::Events() << voiceEvent));

    historyManager.removeEvents(HistoryModel::EventTypeVoice, QDateTime::currentDateTime().addDays(1).toString("yyyy-MM-ddTHH:mm:ss.zzz"), fun);
    QTest::qWait(100);
    QCOMPARE(myEngine.globalObject().property("error").toInt(),HistoryManager::NO_ERROR);
    QCOMPARE(myEngine.globalObject().property("deletedEventCount").toInt(),1);
}

QTEST_MAIN(HistoryManagerTest)
#include "HistoryManagerTest.moc"
