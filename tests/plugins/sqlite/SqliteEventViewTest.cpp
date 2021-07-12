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
#include "sqlitehistoryeventview.h"
#include "textevent.h"
#include "voiceevent.h"
#include "intersectionfilter.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::MatchFlags)

#define EVENT_COUNT 50

class SqliteEventViewTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testNextPage();
    void testFilter();
    void testSort();
    void testSortWithMultipleFields();
    void testFilterWithValueToExclude();
    void testFilterWithFilterLessThan();
    void testFilterWithFilterGreaterThan();
    void testFilterWithFilterGreaterOrEqualsThan();
    void testFilterWithFilterLessOrEqualsThan();

private:
    SQLiteHistoryPlugin *mPlugin;

    void populateDatabase();
};

void SqliteEventViewTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::MatchFlags>();

    qputenv("HISTORY_SQLITE_DBPATH", ":memory:");
    mPlugin = new SQLiteHistoryPlugin(this);

    populateDatabase();
}

void SqliteEventViewTest::testNextPage()
{
    // create a view to return all text threads and check that the right number of items get returned
    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeText);
    QVERIFY(view->IsValid());
    QList<QVariantMap> events = view->NextPage();
    QList<QVariantMap> allEvents;
    while (events.count() > 0) {
        allEvents << events;
        events = view->NextPage();
    }

    QCOMPARE(allEvents.count(), EVENT_COUNT * 2);
    Q_FOREACH(const QVariantMap &event, events) {
        QCOMPARE(event[History::FieldType].toInt(), (int) History::EventTypeText);
    }

    delete view;
}

void SqliteEventViewTest::testFilter()
{
    History::IntersectionFilter filter;
    filter.append(History::Filter(History::FieldAccountId, "account0"));
    filter.append(History::Filter(History::FieldThreadId, "participant0"));
    filter.append(History::Filter(History::FieldEventId, "event21"));

    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeVoice, History::Sort(History::FieldAccountId), filter);
    QVERIFY(view->IsValid());
    QList<QVariantMap> events = view->NextPage();
    QCOMPARE(events.count(), 1);
    QVariantMap event = events.first();
    QCOMPARE(event[History::FieldAccountId].toString(), QString("account0"));
    QCOMPARE(event[History::FieldType].toInt(), (int) History::EventTypeVoice);
    QCOMPARE(event[History::FieldThreadId].toString(), QString("participant0"));
    QCOMPARE(event[History::FieldEventId].toString(), QString("event21"));

    // make sure no more items are returned
    QVERIFY(view->NextPage().isEmpty());
    delete view;
}

void SqliteEventViewTest::testFilterWithValueToExclude()
{
    History::Sort ascendingSort(History::FieldEventId, Qt::AscendingOrder);
    History::IntersectionFilter filter;
    filter.append(History::Filter(History::FieldAccountId, "account0"));
    filter.append(History::Filter(History::FieldMessageStatus, History::MessageStatusTemporarilyFailed, History::MatchNotEquals));
    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeText, ascendingSort, filter);
    QVERIFY(view->IsValid());
    QList<QVariantMap> events = view->NextPage();
    while (!events.isEmpty()) {

        for (const auto& event : events)
        {
            QVERIFY(event[History::FieldMessageStatus].toInt() != (int) History::MessageStatusTemporarilyFailed);
        }

        events = view->NextPage();
    }

    delete view;
}

void SqliteEventViewTest::testFilterWithFilterLessThan()
{
    History::Sort ascendingSort(History::FieldEventId, Qt::AscendingOrder);
    History::IntersectionFilter filter;

    QDateTime queryTime = QDateTime::currentDateTime().addDays(10);
    filter.append(History::Filter(History::FieldAccountId, "account0"));
    filter.append(History::Filter(History::FieldTimestamp, queryTime, History::MatchLess));
    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeText, ascendingSort, filter);
    QVERIFY(view->IsValid());
    QList<QVariantMap> events = view->NextPage();
    while (!events.isEmpty()) {

        for (const auto& event : events)
        {
            QVERIFY(event[History::FieldTimestamp].toDateTime() < queryTime);
        }

        events = view->NextPage();
    }

    delete view;
}

void SqliteEventViewTest::testFilterWithFilterGreaterThan()
{
    History::Sort ascendingSort(History::FieldEventId, Qt::AscendingOrder);
    History::IntersectionFilter filter;

    QDateTime queryTime = QDateTime::currentDateTime().addDays(10);
    filter.append(History::Filter(History::FieldAccountId, "account0"));
    filter.append(History::Filter(History::FieldTimestamp, queryTime, History::MatchGreater));
    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeText, ascendingSort, filter);
    QVERIFY(view->IsValid());
    QList<QVariantMap> events = view->NextPage();
    while (!events.isEmpty()) {

        for (const auto& event : events)
        {
            QVERIFY(event[History::FieldTimestamp].toDateTime() > queryTime);
        }

        events = view->NextPage();
    }

    delete view;
}

void SqliteEventViewTest::testFilterWithFilterGreaterOrEqualsThan()
{
    History::Sort ascendingSort(History::FieldEventId, Qt::AscendingOrder);
    History::IntersectionFilter filter;

    QDateTime queryTime = QDateTime::currentDateTime().addDays(10);
    filter.append(History::Filter(History::FieldAccountId, "account0"));
    filter.append(History::Filter(History::FieldTimestamp, queryTime, History::MatchGreaterOrEquals));
    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeText, ascendingSort, filter);
    QVERIFY(view->IsValid());
    QList<QVariantMap> events = view->NextPage();
    while (!events.isEmpty()) {

        for (const auto& event : events)
        {
            QVERIFY(event[History::FieldTimestamp].toDateTime() >= queryTime);
        }

        events = view->NextPage();
    }

    delete view;
}

void SqliteEventViewTest::testFilterWithFilterLessOrEqualsThan()
{
    History::Sort ascendingSort(History::FieldEventId, Qt::AscendingOrder);
    History::IntersectionFilter filter;

    QDateTime queryTime = QDateTime::currentDateTime().addDays(10);
    filter.append(History::Filter(History::FieldAccountId, "account0"));
    filter.append(History::Filter(History::FieldTimestamp, queryTime, History::MatchLessOrEquals));
    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeText, ascendingSort, filter);
    QVERIFY(view->IsValid());
    QList<QVariantMap> events = view->NextPage();
    while (!events.isEmpty()) {

        for (const auto& event : events)
        {
            QVERIFY(event[History::FieldTimestamp].toDateTime() <= queryTime);
        }

        events = view->NextPage();
    }

    delete view;
}

void SqliteEventViewTest::testSort()
{
    History::Sort ascendingSort(History::FieldEventId, Qt::AscendingOrder);
    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeText, ascendingSort);
    QVERIFY(view->IsValid());
    QList<QVariantMap> allEvents;
    QList<QVariantMap> events = view->NextPage();
    while (!events.isEmpty()) {
        allEvents << events;
        events = view->NextPage();
    }

    QCOMPARE(allEvents.first()[History::FieldEventId].toString(), QString("event00"));
    QCOMPARE(allEvents.last()[History::FieldEventId].toString(), QString("event%1").arg(EVENT_COUNT-1));
    delete view;

    History::Sort descendingSort(History::FieldEventId, Qt::DescendingOrder);
    allEvents.clear();
    view = mPlugin->queryEvents(History::EventTypeVoice, descendingSort);
    QVERIFY(view->IsValid());
    events = view->NextPage();
    while (!events.isEmpty()) {
        allEvents << events;
        events = view->NextPage();
    }

    QCOMPARE(allEvents.first()[History::FieldEventId].toString(), QString("event%1").arg(EVENT_COUNT-1));
    QCOMPARE(allEvents.last()[History::FieldEventId].toString(), QString("event00"));
    delete view;
}

void SqliteEventViewTest::testSortWithMultipleFields()
{
    History::Sort ascendingSort(QString("%1, %2").arg(History::FieldAccountId).arg(History::FieldEventId), Qt::AscendingOrder);
    //History::Sort ascendingSort(QString("%1").arg(History::FieldEventId), Qt::AscendingOrder);
    History::PluginEventView *view = mPlugin->queryEvents(History::EventTypeText, ascendingSort);
    QVERIFY(view->IsValid());
    QList<QVariantMap> allEvents;
    QList<QVariantMap> events = view->NextPage();
    while (!events.isEmpty()) {
        allEvents << events;
        events = view->NextPage();
    }

    QCOMPARE(allEvents[0][History::FieldEventId].toString(), QString("event00"));
    QCOMPARE(allEvents[0][History::FieldAccountId].toString(), QString("account0"));
    QCOMPARE(allEvents[1][History::FieldEventId].toString(), QString("event01"));
    QCOMPARE(allEvents[1][History::FieldAccountId].toString(), QString("account0"));
    delete view;
}

void SqliteEventViewTest::populateDatabase()
{
    mPlugin->beginBatchOperation();

    QDateTime currentDate = QDateTime::currentDateTime();
    // create two threads of each type
    for (int i = 1; i >= 0; --i) {
        QVariantMap voiceThread = mPlugin->createThreadForParticipants(QString("account%1").arg(i),
                                                                       History::EventTypeVoice,
                                                                       QStringList() << QString("participant%1").arg(i));

        // now create some events for this thread
        for (int j = 0; j < EVENT_COUNT; ++j) {
            History::VoiceEvent voiceEvent(voiceThread[History::FieldAccountId].toString(),
                                           voiceThread[History::FieldThreadId].toString(),
                                           QString("event%1").arg(j, 2, 10, QChar('0')),
                                           j % 2 ? "self" : QString("participant%1").arg(i),
                                           currentDate.addDays(j),
                                           j % 2,
                                           j % 2,
                                           j % 2 ? QTime(i, j, 0) : QTime());
            QCOMPARE(mPlugin->writeVoiceEvent(voiceEvent.properties()), History::EventWriteCreated);
        }


        QVariantMap textThread = mPlugin->createThreadForParticipants(QString("account%1").arg(i),
                                                                      History::EventTypeText,
                                                                      QStringList() << QString("participant%1").arg(i));

        for (int j = 0; j < EVENT_COUNT; ++j) {
            History::TextEvent textEvent(textThread[History::FieldAccountId].toString(),
                                         textThread[History::FieldThreadId].toString(),
                                         QString("event%1").arg(j, 2, 10, QChar('0')),
                                         j % 2 ? "self" : QString("participant%1").arg(i),
                                         currentDate.addDays(j),
                                         currentDate.addDays(j).addSecs(-10),
                                         j % 2,
                                         QString("Hello %1").arg(j),
                                         History::MessageTypeText,
                                         j % 2 ? History::MessageStatusDelivered : History::MessageStatusTemporarilyFailed);
            QCOMPARE(mPlugin->writeTextEvent(textEvent.properties()), History::EventWriteCreated);

        }
    }
    mPlugin->endBatchOperation();
}

QTEST_MAIN(SqliteEventViewTest)
#include "SqliteEventViewTest.moc"

