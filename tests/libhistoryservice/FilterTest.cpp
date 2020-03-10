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

#include "filter.h"
#include "intersectionfilter.h"
#include "unionfilter.h"

Q_DECLARE_METATYPE(History::MatchFlags)
Q_DECLARE_METATYPE(History::MatchFlag)
Q_DECLARE_METATYPE(History::Filter)

class FilterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCreateNewFilter_data();
    void testCreateNewFilter();
    void testSetProperties_data();
    void testSetProperties();
    void testToString_data();
    void testToString();
    void testToStringWithNotEqualsMatch();
    void testToStringPrefix();
    void testNullToString();
    void testMatch_data();
    void testMatch();
    void testMatchFlags_data();
    void testMatchFlags();
    void testEqualsOperator();
    void testAssignmentOperator();
    void testIsValid_data();
    void testIsValid();
    void testType();
    void testProperties();
    void testFromProperties();
};

void FilterTest::initTestCase()
{
    qRegisterMetaType<History::MatchFlags>();
    qRegisterMetaType<History::MatchFlag>();
    qRegisterMetaType<History::Filter>();
}

void FilterTest::testCreateNewFilter_data()
{
    QTest::addColumn<QString>("filterProperty");
    QTest::addColumn<QVariant>("filterValue");
    QTest::addColumn<History::MatchFlags>("matchFlags");

    QTest::newRow("string property and value") << "oneProperty" << QVariant("oneValue") << History::MatchFlags();
    QStringList list;
    list << "oneValue" << "anotherValue" << "yetAnotherValue";
    QTest::newRow("a stringlist property") << "stringListProperty" << QVariant(list) << History::MatchFlags(History::MatchCaseSensitive);
    QTest::newRow("combining two flags and an int property") << "intProperty" << QVariant(11)
                                                             << History::MatchFlags(History::MatchPhoneNumber | History::MatchContains);
}

void FilterTest::testCreateNewFilter()
{
    QFETCH(QString, filterProperty);
    QFETCH(QVariant, filterValue);
    QFETCH(History::MatchFlags, matchFlags);

    History::Filter filter(filterProperty, filterValue, matchFlags);
    QCOMPARE(filter.filterProperty(), filterProperty);
    QCOMPARE(filter.filterValue(), filterValue);
    QCOMPARE(filter.matchFlags(), matchFlags);
}

void FilterTest::testSetProperties_data()
{
    QTest::addColumn<QString>("filterProperty");
    QTest::addColumn<QVariant>("filterValue");
    QTest::addColumn<History::MatchFlags>("matchFlags");

    QTest::newRow("string property and value") << "oneProperty" << QVariant("oneValue") << History::MatchFlags();
    QStringList list;
    list << "oneValue" << "anotherValue" << "yetAnotherValue";
    QTest::newRow("a stringlist property") << "stringListProperty" << QVariant(list) << History::MatchFlags(History::MatchCaseSensitive);
    QTest::newRow("combining two flags and an int property") << "intProperty" << QVariant(11)
                                                             << History::MatchFlags(History::MatchPhoneNumber | History::MatchContains);
}

void FilterTest::testSetProperties()
{
    QFETCH(QString, filterProperty);
    QFETCH(QVariant, filterValue);
    QFETCH(History::MatchFlags, matchFlags);

    History::Filter filter;

    filter.setFilterProperty(filterProperty);
    QCOMPARE(filter.filterProperty(), filterProperty);

    filter.setFilterValue(filterValue);
    QCOMPARE(filter.filterValue(), filterValue);

    filter.setMatchFlags(matchFlags);
    QCOMPARE(filter.matchFlags(), matchFlags);
}

void FilterTest::testToString_data()
{
    QTest::addColumn<QString>("filterProperty");
    QTest::addColumn<QVariant>("filterValue");
    QTest::addColumn<QString>("result");

    QTest::newRow("string value") << "stringProperty" << QVariant("stringValue") << "stringProperty=\"stringValue\"";
    QTest::newRow("bool property with false value") << "boolProperty" << QVariant(false) << "boolProperty=0";
    QTest::newRow("bool property with true value") << "boolProperty" << QVariant(true) << "boolProperty=1";
    QTest::newRow("int property") << "intProperty" << QVariant(15) << "intProperty=15";
    QTest::newRow("double property") << "doubleProperty" << QVariant(1.5) << "doubleProperty=1.5";
}

void FilterTest::testToString()
{
    QFETCH(QString, filterProperty);
    QFETCH(QVariant, filterValue);
    QFETCH(QString, result);

    History::Filter filter(filterProperty, filterValue);
    QCOMPARE(filter.toString(), result);
}

void FilterTest::testToStringWithNotEqualsMatch()
{
    QString filterProperty("someProperty");
    QString filterValue("someValue");

    History::Filter filter(filterProperty, filterValue, History::MatchNotEquals);
    QCOMPARE(filter.toString(),QString("%1!=\"%2\"").arg(filterProperty,filterValue));
}

void FilterTest::testToStringPrefix()
{
    QString prefix("somePrefix");
    QString filterProperty("someProperty");
    QString filterValue("someValue");

    History::Filter filter(filterProperty, filterValue);
    QVERIFY(filter.toString(prefix).startsWith(QString("%1.").arg(prefix)));
    QVERIFY(filter.toString().startsWith(filterProperty));
}

void FilterTest::testNullToString()
{
    History::Filter filter;
    QVERIFY(filter.toString().isNull());
}

void FilterTest::testMatch_data()
{
    QTest::addColumn<QVariantMap>("properties");
    QTest::addColumn<QString>("filterProperty");
    QTest::addColumn<QVariant>("filterValue");
    QTest::addColumn<History::MatchFlags>("matchFlags");
    QTest::addColumn<bool>("result");

    QVariantMap map;
    map["stringProperty"] = QString("stringValue");
    QTest::newRow("simple match of a string property") << map << "stringProperty" << QVariant("stringValue")
                                                       << History::MatchFlags(History::MatchCaseSensitive) << true;
    map.clear();

    map["stringProperty"] = QString("anotherValue");
    QTest::newRow("string property that should not match") << map << "stringProperty" << QVariant("stringValue")
                                                           << History::MatchFlags(History::MatchCaseSensitive) << false;
    map.clear();

    map["intProperty"] = 42;
    QTest::newRow("integer property") << map << "intProperty" << QVariant(42) << History::MatchFlags() << true;
    map.clear();

    map["intProperty"] = 41;
    QTest::newRow("integer property that should not match") << map << "intProperty" << QVariant(42) << History::MatchFlags() << false;
    map.clear();

    map["intProperty"] = 42;
    QTest::newRow("empty property") << map << "" << QVariant(42) << History::MatchFlags() << true;
    QTest::newRow("empty value") << map << "intProperty" << QVariant() << History::MatchFlags() << true;

    // FIXME: add more test cases for the match flags once they are implemented
}

void FilterTest::testMatch()
{
    QFETCH(QVariantMap, properties);
    QFETCH(QString, filterProperty);
    QFETCH(QVariant, filterValue);
    QFETCH(History::MatchFlags, matchFlags);
    QFETCH(bool, result);

    History::Filter filter(filterProperty, filterValue, matchFlags);
    QCOMPARE(filter.match(properties), result);

}

void FilterTest::testMatchFlags_data()
{
    QTest::addColumn<History::MatchFlags>("flags");
    QTest::addColumn<History::MatchFlag>("flagToMatch");
    QTest::addColumn<bool>("match");
    /*
    MatchCaseSensitive = 0x01,
    MatchCaseInsensitive = 0x02,
    MatchContains = 0x04,
    MatchPhoneNumber = 0x08,
    MatchNotEquals = 0x16

    */
    QTest::newRow("null flag") << History::MatchFlags() << History::MatchCaseSensitive << false;
    QTest::newRow("case sensitive alone") << History::MatchFlags(History::MatchCaseSensitive) << History::MatchCaseSensitive << true;
    QTest::newRow("case insensitive alone") << History::MatchFlags(History::MatchCaseInsensitive) << History::MatchCaseInsensitive << true;
    QTest::newRow("contains alone") << History::MatchFlags(History::MatchContains) << History::MatchContains << true;
    QTest::newRow("not contains alone") << History::MatchFlags(History::MatchNotEquals) << History::MatchNotEquals << true;
    QTest::newRow("phone number alone") << History::MatchFlags(History::MatchPhoneNumber) << History::MatchPhoneNumber << true;
    QTest::newRow("no mismatch") << History::MatchFlags(History::MatchPhoneNumber) << History::MatchContains << false;
    QTest::newRow("all still match one") << History::MatchFlags(History::MatchCaseInsensitive |
                                                                History::MatchCaseSensitive |
                                                                History::MatchContains |
                                                                History::MatchPhoneNumber) << History::MatchPhoneNumber << true;
}

void FilterTest::testMatchFlags()
{
    QFETCH(History::MatchFlags, flags);
    QFETCH(History::MatchFlag, flagToMatch);
    QFETCH(bool, match);

    QCOMPARE(flags.testFlag(flagToMatch), match);
    QCOMPARE((bool)(flags & flagToMatch), match);
}

void FilterTest::testEqualsOperator()
{
    History::Filter filterOne("oneProperty", "oneValue");
    History::Filter equal("oneProperty", "oneValue");
    History::Filter differentProperty("anotherProperty", "oneValue");
    History::Filter differentValue("oneProperty", "anotherValue");

    QVERIFY(filterOne == equal);
    QVERIFY(!(filterOne == differentProperty));
    QVERIFY(!(filterOne == differentValue));
    QVERIFY(filterOne != differentProperty);
    QVERIFY(filterOne != differentValue);
}

void FilterTest::testAssignmentOperator()
{
    History::Filter filter(History::FieldAccountId, "OneAccountId", History::MatchFlags(History::MatchContains | History::MatchCaseSensitive));
    History::Filter other;
    other = filter;
    QVERIFY(other == filter);
}

void FilterTest::testIsValid_data()
{
    QTest::addColumn<History::Filter>("filter");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("null filter") << History::Filter() << false;
    QTest::newRow("null property") << History::Filter(QString::null, "Foobar") << false;
    QTest::newRow("null value") << History::Filter("oneProperty") << false;
    QTest::newRow("valid filter") << History::Filter("oneProperty", "oneValue") << true;
}

void FilterTest::testIsValid()
{
    QFETCH(History::Filter, filter);
    QFETCH(bool, isValid);
    QCOMPARE(filter.isValid(), isValid);
    QCOMPARE(filter.isNull(), !isValid);
}

void FilterTest::testType()
{
    History::Filter filter;
    QCOMPARE(filter.type(), History::FilterTypeStandard);
}

void FilterTest::testProperties()
{
    // test an empty filter
    History::Filter emptyFilter;
    QVERIFY(emptyFilter.properties().isEmpty());

    // and now a regular filter
    History::Filter filter("foobarProperty", "foobarValue", History::MatchCaseInsensitive);
    QVariantMap properties = filter.properties();
    QCOMPARE(properties[History::FieldFilterType].toInt(), (int)filter.type());
    QCOMPARE(properties[History::FieldFilterProperty].toString(), filter.filterProperty());
    QCOMPARE(properties[History::FieldFilterValue], filter.filterValue());
    QCOMPARE(properties[History::FieldMatchFlags].toInt(), (int)filter.matchFlags());
}

void FilterTest::testFromProperties()
{
    QVariantMap properties;

    // test an empty filter
    History::Filter filter = History::Filter::fromProperties(properties);
    QVERIFY(filter.isNull());

    // and now a regular filter
    properties[History::FieldFilterType] = (int) History::FilterTypeStandard;
    properties[History::FieldFilterProperty] = "oneProperty";
    properties[History::FieldFilterValue] = "oneValue";
    properties[History::FieldMatchFlags] = (int) History::MatchContains;

    filter = History::Filter::fromProperties(properties);
    QCOMPARE(filter.type(), (History::FilterType)properties[History::FieldFilterType].toInt());
    QCOMPARE(filter.filterProperty(), properties[History::FieldFilterProperty].toString());
    QCOMPARE(filter.filterValue(), properties[History::FieldFilterValue]);
    QCOMPARE(filter.matchFlags(), History::MatchFlags(properties[History::FieldMatchFlags].toInt()));

    // test that calling fromProperties() on intersection filters works as expected
    History::IntersectionFilter intersectionFilter;
    intersectionFilter.append(History::Filter("oneProperty", "oneValue"));
    properties = intersectionFilter.properties();
    filter = History::Filter::fromProperties(properties);
    QCOMPARE(filter.type(), History::FilterTypeIntersection);
    QCOMPARE(filter.properties(), properties);

    // and also on union filters
    History::UnionFilter unionFilter;
    unionFilter.append(History::Filter("oneProperty", "oneValue"));
    properties = unionFilter.properties();
    filter = History::Filter::fromProperties(properties);
    QCOMPARE(filter.type(), History::FilterTypeUnion);
    QCOMPARE(filter.properties(), properties);
}

QTEST_MAIN(FilterTest)
#include "FilterTest.moc"
