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

#include "unionfilter.h"

Q_DECLARE_METATYPE(History::MatchFlags)

class UnionFilterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testSetFilters();
    void testAppendFilter();
    void testPrependFilter();
    void testClear();
    void testMatch_data();
    void testMatch();
    void testToStringWithNoFilters();
    void testToStringWithOneFilter();
    void testToStringWithManyFilters();
};

void UnionFilterTest::initTestCase()
{
    qRegisterMetaType<History::MatchFlags>();
}

void UnionFilterTest::testSetFilters()
{
    // create two filters and check that they are properly set
    History::FilterPtr filterOne(new History::Filter("propertyOne", "valueOne"));
    History::FilterPtr filterTwo(new History::Filter("propertyTwo", "valueTwo"));

    History::UnionFilter unionFilter;
    unionFilter.setFilters(History::Filters() << filterOne << filterTwo);

    QCOMPARE(unionFilter.filters().count(), 2);
    QCOMPARE(unionFilter.filters()[0], filterOne);
    QCOMPARE(unionFilter.filters()[1], filterTwo);
}

void UnionFilterTest::testAppendFilter()
{
    // create two filters and check that they are properly set
    History::FilterPtr filterOne(new History::Filter("propertyOne", "valueOne"));
    History::FilterPtr filterTwo(new History::Filter("propertyTwo", "valueTwo"));
    History::FilterPtr filterThree(new History::Filter("propertyThree", "valueThree"));

    History::UnionFilter unionFilter;
    unionFilter.setFilters(History::Filters() << filterOne << filterTwo);
    unionFilter.append(filterThree);

    QCOMPARE(unionFilter.filters().count(), 3);
    QCOMPARE(unionFilter.filters()[2], filterThree);
}

void UnionFilterTest::testPrependFilter()
{
    // create two filters and check that they are properly set
    History::FilterPtr filterOne(new History::Filter("propertyOne", "valueOne"));
    History::FilterPtr filterTwo(new History::Filter("propertyTwo", "valueTwo"));
    History::FilterPtr filterThree(new History::Filter("propertyThree", "valueThree"));

    History::UnionFilter unionFilter;
    unionFilter.setFilters(History::Filters() << filterOne << filterTwo);
    unionFilter.prepend(filterThree);

    QCOMPARE(unionFilter.filters().count(), 3);
    QCOMPARE(unionFilter.filters()[0], filterThree);
}

void UnionFilterTest::testClear()
{
    // create two filters and check that they are properly set
    History::FilterPtr filterOne(new History::Filter("propertyOne", "valueOne"));
    History::FilterPtr filterTwo(new History::Filter("propertyTwo", "valueTwo"));

    History::UnionFilter unionFilter;
    unionFilter.setFilters(History::Filters() << filterOne << filterTwo);
    unionFilter.clear();

    QVERIFY(unionFilter.filters().isEmpty());
}

void UnionFilterTest::testMatch_data()
{
    QTest::addColumn<QVariantMap>("filterProperties");
    QTest::addColumn<QVariantMap>("itemProperties");
    QTest::addColumn<bool>("result");

    // FIXME: take into account the match flags

    QVariantMap filterProperties;
    QVariantMap itemProperties;

    filterProperties["stringProperty"] = QString("stringValue");
    filterProperties["intProperty"] = 10;
    itemProperties = filterProperties;
    QTest::newRow("all matching values") << filterProperties << itemProperties << true;
    itemProperties["intProperty"] = 11;
    QTest::newRow("one of the values match") << filterProperties << itemProperties << true;
    itemProperties["stringProperty"] = QString("noMatch");
    QTest::newRow("no match") << filterProperties << itemProperties << false;
}

void UnionFilterTest::testMatch()
{
    QFETCH(QVariantMap, filterProperties);
    QFETCH(QVariantMap, itemProperties);
    QFETCH(bool, result);

    History::Filters filters;
    Q_FOREACH(const QString &key, filterProperties.keys()) {
        filters << History::FilterPtr(new History::Filter(key, filterProperties[key]));
    }

    History::UnionFilter unionFilter;
    unionFilter.setFilters(filters);

    QCOMPARE(unionFilter.match(itemProperties), result);
}

void UnionFilterTest::testToStringWithNoFilters()
{
    History::UnionFilter filter;
    QVERIFY(filter.toString().isNull());
}

void UnionFilterTest::testToStringWithOneFilter()
{
    // test that with a single filter the result of toString() is equal to the output
    // of calling toString() on the filter directly

    History::FilterPtr filter(new History::Filter("aProperty", "aValue"));
    History::UnionFilter unionFilter;
    unionFilter.append(filter);

    QCOMPARE(unionFilter.toString(), filter->toString());
}

void UnionFilterTest::testToStringWithManyFilters()
{
    // create two filters and check that they are properly set
    History::FilterPtr filterOne(new History::Filter("propertyOne", "valueOne"));
    History::FilterPtr filterTwo(new History::Filter("propertyTwo", "valueTwo"));
    History::FilterPtr filterThree(new History::Filter("propertyThree", "valueThree"));

    History::UnionFilter unionFilter;
    unionFilter.setFilters(History::Filters() << filterOne << filterTwo << filterThree);

    QString stringResult = unionFilter.toString();

    QVERIFY(stringResult.contains(filterOne->toString()));
    QVERIFY(stringResult.contains(filterTwo->toString()));
    QVERIFY(stringResult.contains(filterThree->toString()));
}

QTEST_MAIN(UnionFilterTest)
#include "UnionFilterTest.moc"

