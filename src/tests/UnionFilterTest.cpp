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
Q_DECLARE_METATYPE(History::UnionFilter)

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
    void testConvertToFilterAndBack();
    void testIsValid_data();
    void testIsValid();
    void testProperties();
    void testFromProperties();
};

void UnionFilterTest::initTestCase()
{
    qRegisterMetaType<History::MatchFlags>();
    qRegisterMetaType<History::UnionFilter>();
}

void UnionFilterTest::testSetFilters()
{
    // create two filters and check that they are properly set
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");

    History::UnionFilter unionFilter;
    unionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo);

    QCOMPARE(unionFilter.filters().count(), 2);
    QCOMPARE(unionFilter.filters()[0], filterOne);
    QCOMPARE(unionFilter.filters()[1], filterTwo);
}

void UnionFilterTest::testAppendFilter()
{
    // create two filters and check that they are properly set
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::UnionFilter unionFilter;
    unionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo);
    unionFilter.append(filterThree);

    QCOMPARE(unionFilter.filters().count(), 3);
    QCOMPARE(unionFilter.filters()[2], filterThree);
}

void UnionFilterTest::testPrependFilter()
{
    // create two filters and check that they are properly set
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::UnionFilter unionFilter;
    unionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo);
    unionFilter.prepend(filterThree);

    QCOMPARE(unionFilter.filters().count(), 3);
    QCOMPARE(unionFilter.filters()[0], filterThree);
}

void UnionFilterTest::testClear()
{
    // create two filters and check that they are properly set
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");

    History::UnionFilter unionFilter;
    unionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo);
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
    QTest::newRow("empty match") << QVariantMap() << itemProperties << true;
}

void UnionFilterTest::testMatch()
{
    QFETCH(QVariantMap, filterProperties);
    QFETCH(QVariantMap, itemProperties);
    QFETCH(bool, result);

    QList<History::Filter> filters;
    Q_FOREACH(const QString &key, filterProperties.keys()) {
        filters << History::Filter(key, filterProperties[key]);
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

    History::Filter filter("aProperty", "aValue");
    History::UnionFilter unionFilter;
    unionFilter.append(filter);

    QCOMPARE(unionFilter.toString(), filter.toString());
}

void UnionFilterTest::testToStringWithManyFilters()
{
    // check if all the individual filters are present in the toString() call
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::UnionFilter unionFilter;
    unionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo << filterThree);

    QString stringResult = unionFilter.toString();

    QVERIFY(stringResult.contains(filterOne.toString()));
    QVERIFY(stringResult.contains(filterTwo.toString()));
    QVERIFY(stringResult.contains(filterThree.toString()));
}

void UnionFilterTest::testConvertToFilterAndBack()
{
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::UnionFilter unionFilter;
    unionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo << filterThree);

    History::Filter castFilter = unionFilter;
    QCOMPARE(castFilter.toString(), unionFilter.toString());
    QCOMPARE(castFilter.type(), History::FilterTypeUnion);

    History::UnionFilter andBack = castFilter;
    QCOMPARE(andBack, unionFilter);
    QCOMPARE(andBack.toString(), unionFilter.toString());
}

void UnionFilterTest::testIsValid_data()
{
    QTest::addColumn<History::UnionFilter>("filter");
    QTest::addColumn<bool>("isValid");

    History::UnionFilter filter;
    QTest::newRow("invalid filter") << filter << false;

    filter.append(History::Filter());
    QTest::newRow("valid filter") << filter << true;
}

void UnionFilterTest::testIsValid()
{
    QFETCH(History::UnionFilter, filter);
    QFETCH(bool, isValid);

    QCOMPARE(filter.isValid(), isValid);
}

void UnionFilterTest::testProperties()
{
    History::Filter filterOne("propertyOne", "valueOne");
    History::Filter filterTwo("propertyTwo", "valueTwo");
    History::Filter filterThree("propertyThree", "valueThree");

    History::UnionFilter unionFilter;
    unionFilter.setFilters(QList<History::Filter>() << filterOne << filterTwo << filterThree);

    QVariantMap properties = unionFilter.properties();
    QVERIFY(!properties.isEmpty());
    QVERIFY(properties.contains(History::FieldFilters));
    QCOMPARE(properties[History::FieldFilterType].toInt(), (int) History::FilterTypeUnion);

    QVariantList filters = properties[History::FieldFilters].toList();
    QCOMPARE(filters.count(), unionFilter.filters().count());
    QVariantMap propsOne = filters[0].toMap();
    QCOMPARE(propsOne, filterOne.properties());
    QVariantMap propsTwo = filters[1].toMap();
    QCOMPARE(propsTwo, filterTwo.properties());
    QVariantMap propsThree = filters[2].toMap();
    QCOMPARE(propsThree, filterThree.properties());

    // check that a null filter returns an empty QVariantMap
    History::UnionFilter nullFilter;
    QVERIFY(nullFilter.properties().isEmpty());
}

void UnionFilterTest::testFromProperties()
{
    QVariantMap properties;

    // check that a null filter is returned
    History::Filter nullFilter = History::UnionFilter::fromProperties(properties);
    QVERIFY(nullFilter.isNull());

    properties[History::FieldFilterType] = (int)History::FilterTypeUnion;

    QVariantList filters;
    for (int i = 0; i < 3; ++i) {
        History::Filter filter(QString("filter%1").arg(QString::number(i)), QString("value%1").arg(QString::number(i)), History::MatchCaseInsensitive);
        filters.append(filter.properties());
    }
    properties[History::FieldFilters] = filters;

    History::Filter filter = History::UnionFilter::fromProperties(properties);
    QCOMPARE(filter.type(), History::FilterTypeUnion);
    History::UnionFilter unionFilter = filter;
    QCOMPARE(unionFilter.filters().count(), filters.count());
    for (int i = 0; i < filters.count(); ++i) {
        QCOMPARE(unionFilter.filters()[i].properties(), filters[i].toMap());
    }
}

QTEST_MAIN(UnionFilterTest)
#include "UnionFilterTest.moc"

