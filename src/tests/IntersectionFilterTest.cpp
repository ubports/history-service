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

#include "intersectionfilter.h"

Q_DECLARE_METATYPE(History::MatchFlags)

class IntersectionFilterTest : public QObject
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
    void testConvertToFilterAndBack();
    void testProperties();
    void testFromProperties();
};

void IntersectionFilterTest::initTestCase()
{
    qRegisterMetaType<History::MatchFlags>();
}

void IntersectionFilterTest::testSetFilters()
{
    // create two filters and check that they are properly set
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");

    History::IntersectionFilter intersectionFilter;
    intersectionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo);

    QCOMPARE(intersectionFilter.filters().count(), 2);
    QCOMPARE(intersectionFilter.filters()[0], filterOne);
    QCOMPARE(intersectionFilter.filters()[1], filterTwo);
}

void IntersectionFilterTest::testAppendFilter()
{
    // create two filters and check that they are properly set
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::IntersectionFilter intersectionFilter;
    intersectionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo);
    intersectionFilter.append(filterThree);

    QCOMPARE(intersectionFilter.filters().count(), 3);
    QCOMPARE(intersectionFilter.filters()[2], filterThree);
}

void IntersectionFilterTest::testPrependFilter()
{
    // create two filters and check that they are properly set
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::IntersectionFilter intersectionFilter;
    intersectionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo);
    intersectionFilter.prepend(filterThree);

    QCOMPARE(intersectionFilter.filters().count(), 3);
    QCOMPARE(intersectionFilter.filters()[0], filterThree);
}

void IntersectionFilterTest::testClear()
{
    // create two filters and check that they are properly set
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");

    History::IntersectionFilter intersectionFilter;
    intersectionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo);
    intersectionFilter.clear();

    QVERIFY(intersectionFilter.filters().isEmpty());
}

void IntersectionFilterTest::testMatch_data()
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
    QTest::newRow("one of the values is different") << filterProperties << itemProperties << false;
    itemProperties["stringProperty"] = QString("noMatch");
    QTest::newRow("no match at all") << filterProperties << itemProperties << false;
}

void IntersectionFilterTest::testMatch()
{
    QFETCH(QVariantMap, filterProperties);
    QFETCH(QVariantMap, itemProperties);
    QFETCH(bool, result);

    QList<History::Filter> filters;
    Q_FOREACH(const QString &key, filterProperties.keys()) {
        filters << History::Filter(key, filterProperties[key]);
    }

    History::IntersectionFilter intersectionFilter;
    intersectionFilter.setFilters(filters);

    QCOMPARE(intersectionFilter.match(itemProperties), result);
}

void IntersectionFilterTest::testToStringWithNoFilters()
{
    History::IntersectionFilter filter;
    QVERIFY(filter.toString().isNull());
}

void IntersectionFilterTest::testToStringWithOneFilter()
{
    // test that with a single filter the result of toString() is equal to the output
    // of calling toString() on the filter directly

    History::Filter filter("aProperty", "aValue");
    History::IntersectionFilter intersectionFilter;
    intersectionFilter.append(filter);

    QCOMPARE(intersectionFilter.toString(), filter.toString());
}

void IntersectionFilterTest::testToStringWithManyFilters()
{
    // check if each of the individual filters are present in the toString output
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::IntersectionFilter intersectionFilter;
    intersectionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo << filterThree);

    QString stringResult = intersectionFilter.toString();

    QVERIFY(stringResult.contains(filterOne.toString()));
    QVERIFY(stringResult.contains(filterTwo.toString()));
    QVERIFY(stringResult.contains(filterThree.toString()));
}

void IntersectionFilterTest::testConvertToFilterAndBack()
{
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::IntersectionFilter intersectionFilter;
    intersectionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo << filterThree);

    History::Filter castFilter = intersectionFilter;
    QCOMPARE(castFilter.toString(), intersectionFilter.toString());
    QCOMPARE(castFilter.type(), History::FilterTypeIntersection);

    History::IntersectionFilter andBack = castFilter;
    QCOMPARE(andBack, intersectionFilter);
    QCOMPARE(andBack.toString(), intersectionFilter.toString());
}

void IntersectionFilterTest::testProperties()
{
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::IntersectionFilter intersectionFilter;
    intersectionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo << filterThree);

    QVariantMap properties = intersectionFilter.properties();
    QVERIFY(!properties.isEmpty());
    QVERIFY(properties.contains(History::FieldFilters));
    QCOMPARE(properties[History::FieldFilterType].toInt(), (int) History::FilterTypeIntersection);

    QVariantList filters = properties[History::FieldFilters].toList();
    QCOMPARE(filters.count(), intersectionFilter.filters().count());
    QVariantMap propsOne = filters[0].toMap();
    QCOMPARE(propsOne, filterOne.properties());
    QVariantMap propsTwo = filters[1].toMap();
    QCOMPARE(propsTwo, filterTwo.properties());
    QVariantMap propsThree = filters[2].toMap();
    QCOMPARE(propsThree, filterThree.properties());

    // check that a null filter returns an empty QVariantMap
    History::IntersectionFilter nullFilter;
    QVERIFY(nullFilter.properties().isEmpty());
}

void IntersectionFilterTest::testFromProperties()
{
    QVariantMap properties;

    // check that a null filter is returned
    History::Filter nullFilter = History::IntersectionFilter::fromProperties(properties);
    QVERIFY(nullFilter.isNull());

    properties[History::FieldFilterType] = (int)History::FilterTypeIntersection;

    QVariantList filters;
    for (int i = 0; i < 3; ++i) {
        History::Filter filter(QString("filter%1").arg(QString::number(i)), QString("value%1").arg(QString::number(i)), History::MatchCaseInsensitive);
        filters.append(filter.properties());
    }
    properties[History::FieldFilters] = filters;

    History::Filter filter = History::IntersectionFilter::fromProperties(properties);
    QCOMPARE(filter.type(), History::FilterTypeIntersection);
    History::IntersectionFilter intersectionFilter = filter;
    QCOMPARE(intersectionFilter.filters().count(), filters.count());
    for (int i = 0; i < filters.count(); ++i) {
        QCOMPARE(intersectionFilter.filters()[i].properties(), filters[i].toMap());
    }
}

QTEST_MAIN(IntersectionFilterTest)
#include "IntersectionFilterTest.moc"

