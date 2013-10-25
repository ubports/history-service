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
    void testCopyConstructor();
    void testSetSortProperties_data();
    void testSetSortProperties();
    void testFromProperties_data();
    void testFromProperties();
    void testFromNullProperties();
    void testProperties_data();
    void testProperties();
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

void SortTest::testCopyConstructor()
{
    History::Sort sort(History::FieldCount, Qt::DescendingOrder, Qt::CaseSensitive);
    History::Sort otherSort(sort);

    QCOMPARE(otherSort.sortField(), sort.sortField());
    QCOMPARE(otherSort.sortOrder(), sort.sortOrder());
    QCOMPARE(otherSort.caseSensitivity(), sort.caseSensitivity());
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

void SortTest::testFromProperties_data()
{
    QTest::addColumn<QString>("sortField");
    QTest::addColumn<Qt::SortOrder>("sortOrder");
    QTest::addColumn<Qt::CaseSensitivity>("caseSensitivity");

    QTest::newRow("threadId field ascending case sensitive") << "threadId" << Qt::AscendingOrder << Qt::CaseSensitive;
    QTest::newRow("eventId field descending case insensitive") << "threadId" << Qt::DescendingOrder << Qt::CaseInsensitive;
}

void SortTest::testFromProperties()
{
    QFETCH(QString, sortField);
    QFETCH(Qt::SortOrder, sortOrder);
    QFETCH(Qt::CaseSensitivity, caseSensitivity);

    QVariantMap properties;
    properties[History::FieldSortField] = sortField;
    properties[History::FieldSortOrder] = (int) sortOrder;
    properties[History::FieldCaseSensitivity] = (int) caseSensitivity;

    History::Sort sort = History::Sort::fromProperties(properties);
    sort.setSortField(sortField);
    QCOMPARE(sort.sortField(), sortField);

    sort.setSortOrder(sortOrder);
    QCOMPARE(sort.sortOrder(), sortOrder);

    sort.setCaseSensitivity(caseSensitivity);
    QCOMPARE(sort.caseSensitivity(), caseSensitivity);
}

void SortTest::testFromNullProperties()
{
    History::Sort nullSort;
    History::Sort sort = History::Sort::fromProperties(QVariantMap());

    QCOMPARE(sort.sortField(), nullSort.sortField());
    QCOMPARE(sort.sortOrder(), nullSort.sortOrder());
    QCOMPARE(sort.caseSensitivity(), nullSort.caseSensitivity());
}

void SortTest::testProperties_data()
{
    QTest::addColumn<QString>("sortField");
    QTest::addColumn<Qt::SortOrder>("sortOrder");
    QTest::addColumn<Qt::CaseSensitivity>("caseSensitivity");

    QTest::newRow("threadId field ascending case sensitive") << "threadId" << Qt::AscendingOrder << Qt::CaseSensitive;
    QTest::newRow("eventId field descending case insensitive") << "threadId" << Qt::DescendingOrder << Qt::CaseInsensitive;
}

void SortTest::testProperties()
{
    QFETCH(QString, sortField);
    QFETCH(Qt::SortOrder, sortOrder);
    QFETCH(Qt::CaseSensitivity, caseSensitivity);
    History::Sort sort(sortField, sortOrder, caseSensitivity);

    QVariantMap properties = sort.properties();
    QCOMPARE(properties[History::FieldSortField].toString(), sortField);
    QCOMPARE(properties[History::FieldSortOrder].toInt(), (int) sortOrder);
    QCOMPARE(properties[History::FieldCaseSensitivity].toInt(), (int) caseSensitivity);
}


QTEST_MAIN(SortTest)
#include "SortTest.moc"

