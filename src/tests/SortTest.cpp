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

#include "sort.h"

Q_DECLARE_METATYPE(Qt::SortOrder)
Q_DECLARE_METATYPE(Qt::CaseSensitivity)

class SortTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCreateNewSort_data();
    void testCreateNewSort();
    void testSetSortProperties_data();
    void testSetSortProperties();
};

void SortTest::initTestCase()
{
    qRegisterMetaType<Qt::SortOrder>();
    qRegisterMetaType<Qt::CaseSensitivity>();
}

void SortTest::testCreateNewSort_data()
{
    QTest::addColumn<QString>("sortField");
    QTest::addColumn<Qt::SortOrder>("sortOrder");
    QTest::addColumn<Qt::CaseSensitivity>("caseSensitivity");

    QTest::newRow("threadId field ascending case sensitive") << "threadId" << Qt::AscendingOrder << Qt::CaseSensitive;
    QTest::newRow("eventId field descending case insensitive") << "threadId" << Qt::DescendingOrder << Qt::CaseInsensitive;
}

void SortTest::testCreateNewSort()
{
    QFETCH(QString, sortField);
    QFETCH(Qt::SortOrder, sortOrder);
    QFETCH(Qt::CaseSensitivity, caseSensitivity);

    History::Sort sort(sortField, sortOrder, caseSensitivity);

    QCOMPARE(sort.sortField(), sortField);
    QCOMPARE(sort.sortOrder(), sortOrder);
    QCOMPARE(sort.caseSensitivity(), caseSensitivity);
}

void SortTest::testSetSortProperties_data()
{
    QTest::addColumn<QString>("sortField");
    QTest::addColumn<Qt::SortOrder>("sortOrder");
    QTest::addColumn<Qt::CaseSensitivity>("caseSensitivity");

    QTest::newRow("threadId field ascending case sensitive") << "threadId" << Qt::AscendingOrder << Qt::CaseSensitive;
    QTest::newRow("eventId field descending case insensitive") << "threadId" << Qt::DescendingOrder << Qt::CaseInsensitive;
}

void SortTest::testSetSortProperties()
{
    QFETCH(QString, sortField);
    QFETCH(Qt::SortOrder, sortOrder);
    QFETCH(Qt::CaseSensitivity, caseSensitivity);

    History::Sort sort;

    sort.setSortField(sortField);
    QCOMPARE(sort.sortField(), sortField);

    sort.setSortOrder(sortOrder);
    QCOMPARE(sort.sortOrder(), sortOrder);

    sort.setCaseSensitivity(caseSensitivity);
    QCOMPARE(sort.caseSensitivity(), caseSensitivity);
}


QTEST_MAIN(SortTest)
#include "SortTest.moc"

