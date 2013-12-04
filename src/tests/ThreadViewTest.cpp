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

#include "manager.h"
#include "thread.h"
#include "threadview.h"
#include "textevent.h"
#include "unionfilter.h"
#include "voiceevent.h"

Q_DECLARE_METATYPE(History::EventType)
Q_DECLARE_METATYPE(History::MatchFlags)
Q_DECLARE_METATYPE(History::Threads)
Q_DECLARE_METATYPE(History::Events)

#define THREAD_COUNT 50

class ThreadViewTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testNextPage();
    void testFilter();
    void testSort();

private:
    void populate();

};

void ThreadViewTest::initTestCase()
{
    qRegisterMetaType<History::EventType>();
    qRegisterMetaType<History::MatchFlags>();
    qRegisterMetaType<History::Threads>();
    qRegisterMetaType<History::Events>();

    populate();
}

void ThreadViewTest::testNextPage()
{
    // create a view to return all text threads and check that the right number of items get returned
    History::ThreadViewPtr view = History::Manager::instance()->queryThreads(History::EventTypeText);
    QVERIFY(view->isValid());
    History::Threads threads = view->nextPage();
    History::Threads allThreads;
    while (threads.count() > 0) {
        allThreads << threads;
        threads = view->nextPage();
    }

    QCOMPARE(allThreads.count(), THREAD_COUNT);
    Q_FOREACH(const History::Thread &thread, threads) {
        QCOMPARE(thread.type(), History::EventTypeText);
    }
}

void ThreadViewTest::testFilter()
{
    History::UnionFilter filter;
    filter.append(History::Filter(History::FieldAccountId, "account10"));
    filter.append(History::Filter(History::FieldAccountId, "account35"));

    History::ThreadViewPtr view = History::Manager::instance()->queryThreads(History::EventTypeVoice, History::Sort(History::FieldAccountId), filter);
    QVERIFY(view->isValid());
    History::Threads threads = view->nextPage();
    QCOMPARE(threads.count(), 2);
    QCOMPARE(threads.first().accountId(), QString("account10"));
    QCOMPARE(threads.first().type(), History::EventTypeVoice);
    QCOMPARE(threads.last().accountId(), QString("account35"));
    QCOMPARE(threads.last().type(), History::EventTypeVoice);

    // make sure no more items are returned
    QVERIFY(view->nextPage().isEmpty());
}

void ThreadViewTest::testSort()
{
    History::Sort ascendingSort(History::FieldAccountId, Qt::AscendingOrder);
    History::ThreadViewPtr view = History::Manager::instance()->queryThreads(History::EventTypeText, ascendingSort);
    QVERIFY(view->isValid());
    History::Threads allThreads;
    History::Threads threads = view->nextPage();
    while (!threads.isEmpty()) {
        allThreads << threads;
        threads = view->nextPage();
    }

    QCOMPARE(allThreads.first().accountId(), QString("account00"));
    QCOMPARE(allThreads.last().accountId(), QString("account%1").arg(THREAD_COUNT-1));

    History::Sort descendingSort(History::FieldAccountId, Qt::DescendingOrder);
    allThreads.clear();
    view = History::Manager::instance()->queryThreads(History::EventTypeVoice, descendingSort);
    QVERIFY(view->isValid());
    threads = view->nextPage();
    while (!threads.isEmpty()) {
        allThreads << threads;
        threads = view->nextPage();
    }

    QCOMPARE(allThreads.first().accountId(), QString("account%1").arg(THREAD_COUNT-1));
    QCOMPARE(allThreads.last().accountId(), QString("account00"));
}

void ThreadViewTest::populate()
{
    // create voice threads
    for (int i = 0; i < THREAD_COUNT; ++i) {
        History::Manager::instance()->threadForParticipants(QString("account%1").arg(i, 2, 10, QChar('0')),
                                                            History::EventTypeVoice,
                                                            QStringList() << QString("participant%1").arg(i, 2, 10, QChar('0')),
                                                            History::MatchCaseSensitive,
                                                            true);
    }

    // and the text threads
    for (int i = 0; i < THREAD_COUNT; ++i) {
        History::Manager::instance()->threadForParticipants(QString("account%1").arg(i, 2, 10, QChar('0')),
                                                            History::EventTypeText,
                                                            QStringList() << QString("participant%1").arg(i, 2, 10, QChar('0')),
                                                            History::MatchCaseSensitive,
                                                            true);
    }
}


QTEST_MAIN(ThreadViewTest)
#include "ThreadViewTest.moc"

