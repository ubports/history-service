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
private:
    History::Manager *mManager;
};

void HistoryGroupedThreadsModelTest::initTestCase()
{
    mManager = History::Manager::instance();
}

void HistoryGroupedThreadsModelTest::testCanFetchMore()
{
    HistoryGroupedThreadsModel model;
    QSignalSpy fetchMoreChanged(&model, SIGNAL(canFetchMoreChanged()));

    // create a temporary thread to populate the model
    History::Thread textThread = mManager->threadForParticipants("accountId",
                                                             History::EventTypeText,
                                                             QStringList() << QString("textParticipant"),
                                                             History::MatchCaseSensitive, true);

    // must return false if there is no filter set
    QVERIFY(!model.canFetchMore());

    HistoryQmlFilter *filter = new HistoryQmlFilter(this);
    filter->setFilterProperty(History::FieldAccountId);
    filter->setFilterValue("accountId");

    model.setFilter(filter);
    // force updateQuery() to be called
    model.componentComplete();

    QVERIFY(model.canFetchMore());
    model.fetchMore();
    QTRY_VERIFY(fetchMoreChanged.count() >= 1);
    QVERIFY(!model.canFetchMore());
}

QTEST_MAIN(HistoryGroupedThreadsModelTest)
#include "HistoryGroupedThreadsModelTest.moc"
