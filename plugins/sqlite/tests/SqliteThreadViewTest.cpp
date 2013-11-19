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
#include "textevent.h"
#include "voiceevent.h"
#include "unionfilter.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::MatchFlags)

#define THREAD_COUNT 50

class SqliteThreadViewTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testNextPage();
    void testFilter();
    void testSort();

private:
    SQLiteHistoryPlugin *mPlugin;

    void populateDatabase();
};

void SqliteThreadViewTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::MatchFlags>();

    qputenv("HISTORY_SQLITE_DBPATH", ":memory:");
    mPlugin = new SQLiteHistoryPlugin(this);

    populateDatabase();
}

void SqliteThreadViewTest::testNextPage()
{
    // create a view to return all text threads and check that the right number of items get returned
    History::PluginThreadView *view = mPlugin->queryThreads(History::EventTypeText);
    QVERIFY(view->IsValid());
    QList<QVariantMap> threads = view->NextPage();
    QList<QVariantMap> allThreads;
    while (threads.count() > 0) {
        allThreads << threads;
        threads = view->NextPage();
    }

    QCOMPARE(allThreads.count(), THREAD_COUNT);
    Q_FOREACH(const QVariantMap &thread, threads) {
        QCOMPARE(thread[History::FieldType].toInt(), (int) History::EventTypeText);
    }

    delete view;
}

void SqliteThreadViewTest::testFilter()
{
    History::UnionFilter filter;
    filter.append(History::Filter(History::FieldAccountId, "account10"));
    filter.append(History::Filter(History::FieldAccountId, "account35"));

    History::PluginThreadView *view = mPlugin->queryThreads(History::EventTypeVoice, History::Sort(History::FieldAccountId), filter);
    QVERIFY(view->IsValid());
    QList<QVariantMap> threads = view->NextPage();
    QCOMPARE(threads.count(), 2);
    QCOMPARE(threads[0][History::FieldAccountId].toString(), QString("account10"));
    QCOMPARE(threads[0][History::FieldType].toInt(), (int) History::EventTypeVoice);
    QCOMPARE(threads[1][History::FieldAccountId].toString(), QString("account35"));
    QCOMPARE(threads[0][History::FieldType].toInt(), (int) History::EventTypeVoice);

    // make sure no more items are returned
    QVERIFY(view->NextPage().isEmpty());
    delete view;
}

void SqliteThreadViewTest::testSort()
{
    History::Sort ascendingSort(History::FieldAccountId, Qt::AscendingOrder);
    History::PluginThreadView *view = mPlugin->queryThreads(History::EventTypeText, ascendingSort);
    QVERIFY(view->IsValid());
    QList<QVariantMap> allThreads;
    QList<QVariantMap> threads = view->NextPage();
    while (!threads.isEmpty()) {
        allThreads << threads;
        threads = view->NextPage();
    }

    QCOMPARE(allThreads.first()[History::FieldAccountId].toString(), QString("account00"));
    QCOMPARE(allThreads.last()[History::FieldAccountId].toString(), QString("account%1").arg(THREAD_COUNT-1));
    delete view;

    History::Sort descendingSort(History::FieldAccountId, Qt::DescendingOrder);
    allThreads.clear();
    view = mPlugin->queryThreads(History::EventTypeVoice, descendingSort);
    QVERIFY(view->IsValid());
    threads = view->NextPage();
    while (!threads.isEmpty()) {
        allThreads << threads;
        threads = view->NextPage();
    }

    QCOMPARE(allThreads.first()[History::FieldAccountId].toString(), QString("account%1").arg(THREAD_COUNT-1));
    QCOMPARE(allThreads.last()[History::FieldAccountId].toString(), QString("account00"));
    delete view;

}

void SqliteThreadViewTest::populateDatabase()
{
    mPlugin->beginBatchOperation();
    // create voice threads
    for (int i = 0; i < THREAD_COUNT; ++i) {
        mPlugin->createThreadForParticipants(QString("account%1").arg(i, 2, 10, QChar('0')),
                                             History::EventTypeVoice,
                                             QStringList() << QString("participant%1").arg(i, 2, 10, QChar('0')));
    }

    // and the text threads
    for (int i = 0; i < THREAD_COUNT; ++i) {
        mPlugin->createThreadForParticipants(QString("account%1").arg(i, 2, 10, QChar('0')),
                                             History::EventTypeText,
                                             QStringList() << QString("participant%1").arg(i, 2, 10, QChar('0')));
    }
    mPlugin->endBatchOperation();
}

QTEST_MAIN(SqliteThreadViewTest)
#include "SqliteThreadViewTest.moc"

