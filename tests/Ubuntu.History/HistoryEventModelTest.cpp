/*
 * Copyright (C) 2016-2017 Canonical, Ltd.
 *
 * This file is part of history-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest/QtTest>
#include "telepathytest.h"
#include "manager.h"
#include "textevent.h"
#include "historyeventmodel.h"

class HistoryEventModelTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testTelepathyInitializedCorrectly();

private:
    History::Manager *mManager;
};

void HistoryEventModelTest::initTestCase()
{
    initialize(0);

    mManager = History::Manager::instance();
}

void HistoryEventModelTest::testTelepathyInitializedCorrectly()
{
    Tp::AccountPtr account = addAccount("mock", "ofono", "My Account");
    QVERIFY(!account.isNull());

    // we don't actually test anything on the model other than it having the event
    // but if telepathy is not properly initialized, this will crash.
    HistoryEventModel model;
    QString participant("textParticipant");

    // create a temporary thread to populate the model
    History::Thread textThread = mManager->threadForParticipants(account->uniqueIdentifier(),
                                                             History::EventTypeText,
                                                             QStringList() << participant,
                                                             History::MatchCaseSensitive, true);

    History::TextEvent event(textThread.accountId(),
                             textThread.threadId(),
                             "theEventId",
                             participant,
                             QDateTime::currentDateTime(),
                             true,
                             "Hi there",
                             History::MessageTypeText,
                             History::MessageStatusRead,
                             QDateTime::currentDateTime(),
                             "The subject",
                             History::InformationTypeNone,
                             History::TextEventAttachments(),
                             textThread.participants());
    QVERIFY(mManager->writeEvents(History::Events() << event));

    HistoryQmlFilter *filter = new HistoryQmlFilter(this);
    filter->setFilterProperty(History::FieldThreadId);
    filter->setFilterValue(textThread.threadId());
    model.setFilter(filter);

    HistoryQmlSort *sort = new HistoryQmlSort(this);
    sort->setSortOrder(HistoryQmlSort::DescendingOrder);
    sort->setSortField("timestamp");
    model.setSort(sort);

    QTRY_COMPARE(model.rowCount(), 1);

    // this will trigger the crash if tp-qt is not properly initialized
    qDebug() << model.index(0).data(HistoryEventModel::SenderRole);

    mManager->removeThreads(History::Threads() << textThread);
    QTRY_COMPARE(model.rowCount(), 0);
    
    //Cleanup
    delete sort;
    delete filter;
}

QTEST_MAIN(HistoryEventModelTest)
#include "HistoryEventModelTest.moc"
