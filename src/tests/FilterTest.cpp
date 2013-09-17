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

Q_DECLARE_METATYPE(History::MatchFlags)

class FilterTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCreateNewFilter_data();
    void testCreateNewFilter();
    void testSetProperties_data();
    void testSetProperties();
    void testToStringPrefix();
    void testMatch_data();
    void testMatch();
    void testEquals();
};

void FilterTest::initTestCase()
{
    qRegisterMetaType<History::MatchFlags>();
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

void FilterTest::testToStringPrefix()
{
    QString prefix("somePrefix");
    QString filterProperty("someProperty");
    QString filterValue("someValue");

    History::Filter filter(filterProperty, filterValue);
    QVERIFY(filter.toString(prefix).startsWith(QString("%1.").arg(prefix)));
    QVERIFY(filter.toString().startsWith(filterProperty));
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

void FilterTest::testEquals()
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


QTEST_MAIN(FilterTest)
#include "FilterTest.moc"
