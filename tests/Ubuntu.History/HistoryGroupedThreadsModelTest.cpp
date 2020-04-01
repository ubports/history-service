/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include "manager.h"
#include "historygroupedthreadsmodel.h"

class HistoryGroupedThreadsModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCanFetchMore();
    void testThreadsUpdated();
    void cleanupTestCase();
private:
    History::Manager *mManager;
};

void HistoryGroupedThreadsModelTest::initTestCase()
{
    mManager = History::Manager::instance();
}

void HistoryGroupedThreadsModelTest::cleanupTestCase()
{
    delete mManager;
}

void HistoryGroupedThreadsModelTest::testCanFetchMore()
{

    HistoryGroupedThreadsModel model;
    QSignalSpy fetchMoreChanged(&model, SIGNAL(canFetchMoreChanged()));

    // create a temporary thread to populate the model
    History::Thread textThread = mManager->threadForParticipants("accountId0",
                                                             History::EventTypeText,
                                                             QStringList() << QString("textParticipant"),
                                                             History::MatchCaseSensitive, true);

    // must return false if there is no filter set
    QVERIFY(!model.canFetchMore());

    HistoryQmlFilter *filter = new HistoryQmlFilter(this);
    model.setFilter(filter);
    model.setGroupingProperty(History::FieldParticipants);

    HistoryQmlSort *sort = new HistoryQmlSort(this);
    sort->setSortOrder(HistoryQmlSort::DescendingOrder);
    sort->setSortField("lastEventTimestamp");
    model.setSort(sort);

    // force updateQuery() to be called
    model.componentComplete();

    QVERIFY(model.canFetchMore());
    model.fetchMore();
    QTRY_VERIFY(fetchMoreChanged.count() >= 1);
    QVERIFY(!model.canFetchMore());

    QTRY_COMPARE(model.rowCount(), 1);
    mManager->removeThreads(History::Threads() << textThread);
    QTRY_COMPARE(model.rowCount(), 0);

}

void HistoryGroupedThreadsModelTest::testThreadsUpdated()
{

    HistoryGroupedThreadsModel model;
    QSignalSpy dataChanged(&model, SIGNAL(dataChanged(QModelIndex, QModelIndex)));
    QSignalSpy rowsRemoved(&model, SIGNAL(rowsRemoved(QModelIndex, int, int)));
/*
    HistoryQmlFilter *filter = new HistoryQmlFilter(this);
    model.setFilter(filter);
    model.setGroupingProperty(History::FieldParticipants);

    HistoryQmlSort *sort = new HistoryQmlSort(this);
    sort->setSortOrder(HistoryQmlSort::DescendingOrder);
    sort->setSortField("lastEventTimestamp");
    model.setSort(sort);

    // force updateQuery() to be called
    model.componentComplete();

    // create first thread
    History::Thread textThread = mManager->threadForParticipants("ofono/ofono/account0",
                                                             History::EventTypeText,
                                                             QStringList() << QString("1234567"),
                                                             History::MatchPhoneNumber, true);

    // insert one event
    History::TextEvent firstEvent = History::TextEvent(textThread.accountId(), textThread.threadId(), QString("eventId1%1").arg(QString::number(qrand() % 1024)),
                                   QString("1234567"), QDateTime::currentDateTime(), false, "Random Message",
                                   History::MessageTypeText);
    mManager->writeEvents(History::Events() << firstEvent);
    QTRY_COMPARE(dataChanged.count(), 1);

    QModelIndex firstIndex = dataChanged.first().first().value<QModelIndex>();
    QString lastEventMessage = model.data(firstIndex, HistoryThreadModel::LastEventTextMessageRole).toString();
    QCOMPARE(QString("Random Message"), lastEventMessage);
    dataChanged.clear();
 
    // create another thread to be grouped, but using another kind of account
    textThread = mManager->threadForParticipants("multimedia/multimedia/account1",
                                                 History::EventTypeText,
                                                 QStringList() << QString("1234567"),
                                                 History::MatchPhoneNumber, true);

    QTRY_COMPARE(dataChanged.count(), 1);
    QModelIndex index = dataChanged.first().first().value<QModelIndex>();
    QCOMPARE(firstIndex, index);
    dataChanged.clear();

    // insert another event in second thread
    History::TextEvent secondEvent = History::TextEvent(textThread.accountId(), textThread.threadId(), QString("eventId2%1").arg(QString::number(qrand() % 1024)),
                               QString("1234567"), QDateTime::currentDateTime().addSecs(1), false, "Random Message2",
                               History::MessageTypeText);
    mManager->writeEvents(History::Events() << secondEvent);
    QTRY_COMPARE(dataChanged.count(), 1);

    // make sure the index is the same, meaning both threads are grouped
    index = dataChanged.first().first().value<QModelIndex>();
    QCOMPARE(firstIndex, index);

    // check if latest message is from the second event
    lastEventMessage = model.data(index, HistoryThreadModel::LastEventTextMessageRole).toString();

    // check if count is correct given that we have two threads grouped with one message in each
    QCOMPARE(model.data(index, HistoryThreadModel::CountRole).toInt(), 2);
    QCOMPARE(QString("Random Message2"), lastEventMessage);
    dataChanged.clear();

    // delete latest event and make sure the text displayed is from the first thread again
    mManager->removeEvents(History::Events() << secondEvent);
    QTRY_COMPARE(dataChanged.count(), 1);
    index = dataChanged.first().first().value<QModelIndex>();
    QCOMPARE(firstIndex, index);
    lastEventMessage = model.data(index, HistoryThreadModel::LastEventTextMessageRole).toString();
    QCOMPARE(QString("Random Message"), lastEventMessage);
    // check if count is correct given that we have only one thread now
    QCOMPARE(model.data(index, HistoryThreadModel::CountRole).toInt(), 1);
 
    // delete first event and make sure the model is cleared
    mManager->removeEvents(History::Events() << firstEvent);
    QTRY_COMPARE(rowsRemoved.count(), 1);
    QTRY_COMPARE(model.rowCount(), 0);
*/
}

QTEST_MAIN(HistoryGroupedThreadsModelTest)
#include "HistoryGroupedThreadsModelTest.moc"
