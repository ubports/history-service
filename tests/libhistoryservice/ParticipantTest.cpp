/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include "participant.h"
#include "types.h"

Q_DECLARE_METATYPE(History::Participant)

class ParticipantTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testConstructor();
    void testNullParticipant();
    void testIsNull_data();
    void testIsNull();
    void testCopyConstructor();
    void testAssignmentOperator();
    void testEqualsOperator_data();
    void testEqualsOperator();
    void testProperties();
    void testFromProperties();
    void testIdentifiers();
    void testFromVariantList();
    void testToVariantList();
    void testFromVariantWithVariantList();
};

void ParticipantTest::initTestCase()
{
    qRegisterMetaType<History::Participant>();
}

void ParticipantTest::testConstructor()
{
    QString accountId("theAccountId");
    QString identifier("theParticipantId");
    QString contactId("theContactId");
    QString alias("theAlias");
    QString avatar("theAvatar");
    uint state = History::ParticipantStateRegular;
    QVariantMap detailProperties;
    detailProperties["someProperty"] = "someValue";


    History::Participant participant(accountId, identifier, contactId, alias, avatar, state, detailProperties);
    QCOMPARE(participant.accountId(), accountId);
    QCOMPARE(participant.identifier(), identifier);
    QCOMPARE(participant.contactId(), contactId);
    QCOMPARE(participant.alias(), alias);
    QCOMPARE(participant.avatar(), avatar);
    QCOMPARE(participant.state(), state);
    QCOMPARE(participant.detailProperties(), detailProperties);
}

void ParticipantTest::testNullParticipant()
{
    // check that a null participant returns true
    History::Participant nullParticipant;
    QVERIFY(nullParticipant.isNull());
}

void ParticipantTest::testIsNull_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("identifier");
    QTest::addColumn<bool>("isNull");

    QTest::newRow("all null") << QString() << QString() << true;
    QTest::newRow("null accountId") << QString() << "some identifier" << true;
    QTest::newRow("null identifier") << "some account ID" << QString() << true;
    QTest::newRow("valid account and identifier") << "theAccountId" << "theIdentifier" << false;
}

void ParticipantTest::testIsNull()
{
    QFETCH(QString, accountId);
    QFETCH(QString, identifier);
    QFETCH(bool, isNull);

    History::Participant participant(accountId, identifier);
    QCOMPARE(participant.isNull(), isNull);

}

void ParticipantTest::testCopyConstructor()
{
    QVariantMap detailProperties;
    detailProperties["theProperty"] = "theValue";
    History::Participant original("accountId", "identifier", "contactId", "alias", "avatar", History::ParticipantStateRegular, detailProperties);

    History::Participant copy(original);

    QCOMPARE(copy.accountId(), original.accountId());
    QCOMPARE(copy.identifier(), original.identifier());
    QCOMPARE(copy.contactId(), original.contactId());
    QCOMPARE(copy.alias(), original.alias());
    QCOMPARE(copy.avatar(), original.avatar());
    QCOMPARE(copy.state(), original.state());
    QCOMPARE(copy.detailProperties(), original.detailProperties());
}

void ParticipantTest::testAssignmentOperator()
{
    QVariantMap detailProperties;
    detailProperties["theProperty2"] = "theValue2";
    History::Participant original("accountId2", "identifier2", "contactId2", "alias2", "avatar2", History::ParticipantStateRegular, detailProperties);

    History::Participant copy;
    copy = original;

    QCOMPARE(copy.accountId(), original.accountId());
    QCOMPARE(copy.identifier(), original.identifier());
    QCOMPARE(copy.contactId(), original.contactId());
    QCOMPARE(copy.alias(), original.alias());
    QCOMPARE(copy.avatar(), original.avatar());
    QCOMPARE(copy.state(), original.state());
    QCOMPARE(copy.detailProperties(), original.detailProperties());
}

void ParticipantTest::testEqualsOperator_data()
{
    QTest::addColumn<QString>("accountId1");
    QTest::addColumn<QString>("identifier1");
    QTest::addColumn<QString>("accountId2");
    QTest::addColumn<QString>("identifier2");
    QTest::addColumn<bool>("equals");

    QTest::newRow("same participant") << "theAccountId" << "theIdentifier" << "theAccountId" << "theIdentifier" << true;
    QTest::newRow("different identifiers") << "theAccountId" << "theIdentifier1" << "theAccountId" << "theIdentifier2" << false;
    QTest::newRow("different accounts") << "theAccountId1" << "theIdentifier" << "theAccountId2" << "theIdentifier" << false;
    QTest::newRow("all different") << "theAccountId" << "theIdentifier" << "theAccountId2" << "theIdentifier2" << false;
}

void ParticipantTest::testEqualsOperator()
{
    QFETCH(QString, accountId1);
    QFETCH(QString, identifier1);
    QFETCH(QString, accountId2);
    QFETCH(QString, identifier2);
    QFETCH(bool, equals);

    History::Participant participant1(accountId1, identifier1);
    History::Participant participant2(accountId2, identifier2);
    QCOMPARE((participant1 == participant2), equals);
}

void ParticipantTest::testProperties()
{
    QVariantMap detailProperties;
    detailProperties["someDetailProperty"] = "someValue";

    History::Participant participant("theAccountId", "theIdentifier", "theContactId", "theAlias", "theAvatar", History::ParticipantStateRegular, detailProperties);
    QVariantMap properties = participant.properties();
    QCOMPARE(properties[History::FieldAccountId].toString(), participant.accountId());
    QCOMPARE(properties[History::FieldIdentifier].toString(), participant.identifier());
    QCOMPARE(properties[History::FieldContactId].toString(), participant.contactId());
    QCOMPARE(properties[History::FieldAlias].toString(), participant.alias());
    QCOMPARE(properties[History::FieldAvatar].toString(), participant.avatar());
    QCOMPARE(properties[History::FieldParticipantState].toUInt(), participant.state());
    QCOMPARE(properties[History::FieldDetailProperties].toMap(), participant.detailProperties());
}

void ParticipantTest::testFromProperties()
{
    QVariantMap properties;
    QVariantMap detailProperties;

    properties[History::FieldAccountId] = "someAccountId";
    properties[History::FieldIdentifier] = "someIdentifier";
    properties[History::FieldContactId] = "someContactId";
    properties[History::FieldAlias] = "someAlias";
    properties[History::FieldAvatar] = "someAvatar";
    properties[History::FieldParticipantState] = History::ParticipantStateRegular;
    detailProperties["someDetailProperty"] = "someValue";
    properties[History::FieldDetailProperties] = detailProperties;

    History::Participant participant = History::Participant::fromProperties(properties);
    QCOMPARE(participant.accountId(), properties[History::FieldAccountId].toString());
    QCOMPARE(participant.identifier(), properties[History::FieldIdentifier].toString());
    QCOMPARE(participant.contactId(), properties[History::FieldContactId].toString());
    QCOMPARE(participant.alias(), properties[History::FieldAlias].toString());
    QCOMPARE(participant.avatar(), properties[History::FieldAvatar].toString());
    QCOMPARE(participant.state(), properties[History::FieldParticipantState].toUInt());
    QCOMPARE(participant.detailProperties(), properties[History::FieldDetailProperties].toMap());
}

void ParticipantTest::testIdentifiers()
{
    QStringList identifiers;
    identifiers << "firstId" << "secondId" << "thirdId";
    History::Participants participants;
    Q_FOREACH(const QString &identifier, identifiers) {
        participants << History::Participant("theAccountId", identifier);
    }
    QCOMPARE(participants.identifiers(), identifiers);
}

void ParticipantTest::testFromVariantList()
{
    QVariantList list;
    for (int i = 0; i < 10; ++i) {
        list << History::Participant("theAccountId", QString("identifier%1").arg(QString::number(i))).properties();
    }

    History::Participants participants = History::Participants::fromVariantList(list);
    QCOMPARE(participants.count(), list.count());
    for (int i = 0; i < participants.count(); ++i) {
        QCOMPARE(participants[i].properties(), list[i].toMap());
    }
}

void ParticipantTest::testToVariantList()
{
    History::Participants participants;
    for (int i = 0; i < 10; ++i) {
        participants << History::Participant("theAccountId", QString("identifier%1").arg(QString::number(i)));
    }

    QVariantList list = participants.toVariantList();
    QCOMPARE(list.count(), participants.count());
    for (int i = 0; i < list.count(); ++i) {
        QCOMPARE(list[i].toMap(), participants[i].properties());
    }
}

void ParticipantTest::testFromVariantWithVariantList()
{
    QVariantList list;
    for (int i = 0; i < 10; ++i) {
        list << History::Participant("theAccountId", QString("identifier%1").arg(QString::number(i))).properties();
    }

    History::Participants participants = History::Participants::fromVariant(list);
    QCOMPARE(participants.count(), list.count());
    for (int i = 0; i < participants.count(); ++i) {
        QCOMPARE(participants[i].properties(), list[i].toMap());
    }
}

QTEST_MAIN(ParticipantTest)
#include "ParticipantTest.moc"
