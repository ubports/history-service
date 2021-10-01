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
    QSignalSpy countChanged(&historyManager, SIGNAL(countChanged()));
    // no type
    QCOMPARE(historyManager.removeAll(History::EventTypeNull, QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz")), false);
    QCOMPARE(historyManager.error(),HistoryManager::OPERATION_INVALID);
    //historyManager.setType(HistoryModel::EventTypeVoice);

    // bad filter
    QCOMPARE(historyManager.removeAll(HistoryModel::EventTypeVoice, ""), false);
    QCOMPARE(historyManager.error(),HistoryManager::OPERATION_INVALID);

    //start remove no datas
    QCOMPARE(historyManager.removeAll(HistoryModel::EventTypeVoice, QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz")), false);
    QTRY_COMPARE(countChanged.count(), 1);
    QCOMPARE(historyManager.count(), 0);
    QCOMPARE(historyManager.error(),HistoryManager::NO_ERROR);
}

void HistoryManagerTest::testRemoveAll()
{
    HistoryManager historyManager;
    QSignalSpy countChanged(&historyManager, SIGNAL(countChanged()));
    QSignalSpy operationEnded(&historyManager, SIGNAL(operationEnded()));
    QSignalSpy operationStarted(&historyManager, SIGNAL(operationStarted()));
    QSignalSpy deletedCountChanged(&historyManager, SIGNAL(deletedCountChanged()));

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
                                   true,
                                   true);
    QVERIFY(mManager->writeEvents(History::Events() << voiceEvent));

    QVERIFY(historyManager.removeAll(HistoryModel::EventTypeVoice, QDateTime::currentDateTime().addDays(1).toString("yyyy-MM-ddTHH:mm:ss.zzz")));
    QTRY_COMPARE(operationStarted.count(), 1);
    QTRY_COMPARE(operationEnded.count(), 1);
    QCOMPARE(historyManager.deletedCount(), 1);
    QCOMPARE(historyManager.error(),HistoryManager::NO_ERROR);
}


QTEST_MAIN(HistoryManagerTest)
#include "HistoryManagerTest.moc"
