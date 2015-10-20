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
#include <QContactManager>
#include <QContact>
#include <QContactExtendedDetail>
#include <QContactPhoneNumber>
#include <QContactName>

#include "telepathytest.h"
#include "contactmatcher_p.h"
#include "types.h"
#include "phoneutils_p.h"

QTCONTACTS_USE_NAMESPACE

class ContactMatcherTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void clean();
    void testMatchExistingContact_data();
    void testMatchExistingContact();
    void testContactAdded();
    void testContactRemoved();
    void testSynchronousContactInfoRequest();
    void testWatchIdentifier();

protected:
    QContact createContact(const QString &firstName, const QString &lastName, const QStringList &phoneNumbers = QStringList(), const QStringList &extendedDetails = QStringList());

private:
    QContactManager *mContactManager;
    QContact mPhoneContact;
    QContact mExtendedContact;
};

void ContactMatcherTest::initTestCase()
{
    initialize();
    mContactManager = new QContactManager("memory");
    ContactMatcher::instance(mContactManager);

    // create two contacts to test
    mPhoneContact = createContact("Phone", "Contact", QStringList() << "123456789" << "7654321");
    mExtendedContact = createContact("Extended", "Generic Contact", QStringList(), QStringList() << "123456789");
}

void ContactMatcherTest::init()
{
    // just add two telepathy mock accounts to make sure we get the addressable fields correctly
    addAccount("mock", "mock", "generic account");
    addAccount("mock", "ofono", "phone account");
}

void ContactMatcherTest::clean()
{
    doCleanup();
}

void ContactMatcherTest::testMatchExistingContact_data()
{
    QTest::addColumn<QString>("accountId");
    QTest::addColumn<QString>("identifier");
    QTest::addColumn<QString>("contactId");
    QTest::addColumn<bool>("phoneNumberCompare");

    QTest::newRow("match exact phone id") << QString("mock/ofono/account0") << QString("123456789") << mPhoneContact.id().toString() << false;
    QTest::newRow("match phone number with prefix") << QString("mock/ofono/account0") << QString("+10987654321") << mPhoneContact.id().toString() << true;
    QTest::newRow("match exact extra id") << QString("mock/mock/account0") << QString("123456789") << mExtendedContact.id().toString() << false;
}

void ContactMatcherTest::testMatchExistingContact()
{
    QFETCH(QString, accountId);
    QFETCH(QString, identifier);
    QFETCH(QString, contactId);
    QFETCH(bool, phoneNumberCompare);

    QSignalSpy contactInfoSpy(ContactMatcher::instance(), SIGNAL(contactInfoChanged(QString,QString,QVariantMap)));
    QVariantMap info = ContactMatcher::instance()->contactInfo(accountId, identifier);
    if (phoneNumberCompare) {
        QVERIFY(PhoneUtils::comparePhoneNumbers(info[History::FieldIdentifier].toString(), identifier));
    } else {
        QCOMPARE(info[History::FieldIdentifier].toString(), identifier);
    }

    QTRY_COMPARE(contactInfoSpy.count(), 1);
    QCOMPARE(contactInfoSpy.first()[0].toString(), accountId);
    QCOMPARE(contactInfoSpy.first()[1].toString(), identifier);
    info = contactInfoSpy.first()[2].toMap();
    QCOMPARE(info[History::FieldContactId].toString(), contactId);
    if (phoneNumberCompare) {
        QVERIFY(PhoneUtils::comparePhoneNumbers(info[History::FieldIdentifier].toString(), identifier));
    } else {
        QCOMPARE(info[History::FieldIdentifier].toString(), identifier);
    }
}

void ContactMatcherTest::testContactAdded()
{
    QSignalSpy contactInfoSpy(ContactMatcher::instance(), SIGNAL(contactInfoChanged(QString,QString,QVariantMap)));
    QString identifier("5555555");
    QString accountId("mock/ofono/account0");
    QVariantMap info = ContactMatcher::instance()->contactInfo(accountId, identifier);
    QCOMPARE(info[History::FieldIdentifier].toString(), identifier);
    QVERIFY(!info.contains(History::FieldContactId));

    // now add a contact that matches this item
    QContact contact = createContact("Added", "Contact", QStringList() << identifier);
    QTRY_COMPARE(contactInfoSpy.count(), 1);
    QCOMPARE(contactInfoSpy.first()[0].toString(), accountId);
    QCOMPARE(contactInfoSpy.first()[1].toString(), identifier);
    QCOMPARE(contactInfoSpy.first()[2].toMap()[History::FieldContactId].toString(), contact.id().toString());
}

void ContactMatcherTest::testContactRemoved()
{
    QSignalSpy contactInfoSpy(ContactMatcher::instance(), SIGNAL(contactInfoChanged(QString,QString,QVariantMap)));
    QString identifier("6666666");
    QString accountId("mock/ofono/account0");
    QVariantMap info = ContactMatcher::instance()->contactInfo(accountId, identifier);
    QCOMPARE(info[History::FieldIdentifier].toString(), identifier);

    // now add a contact that matches this item
    QContact contact = createContact("Removed", "Contact", QStringList() << identifier);
    QTRY_COMPARE(contactInfoSpy.count(), 1);

    // now that the contact info is filled, remove the contact
    contactInfoSpy.clear();
    QVERIFY(mContactManager->removeContact(contact.id()));
    QTRY_COMPARE(contactInfoSpy.count(), 1);
    QCOMPARE(contactInfoSpy.first()[0].toString(), accountId);
    QCOMPARE(contactInfoSpy.first()[1].toString(), identifier);
    QVERIFY(!contactInfoSpy.first()[2].toMap().contains(History::FieldContactId));
}

void ContactMatcherTest::testSynchronousContactInfoRequest()
{
    QString identifier("77777777");
    QString accountId("mock/ofono/account0");

    // now add a contact that matches this item
    QContact contact = createContact("Synchronous", "Contact", QStringList() << identifier);

    // now that the contact info is filled, remove the contact
    QVariantMap info = ContactMatcher::instance()->contactInfo(accountId, identifier, true);
    QCOMPARE(info[History::FieldIdentifier].toString(), identifier);
    QCOMPARE(info[History::FieldAccountId].toString(), accountId);
    QVERIFY(!info[History::FieldContactId].toString().isEmpty());

    // and remove this contact to not interfere in the other tests
    QVERIFY(mContactManager->removeContact(contact.id()));
}

void ContactMatcherTest::testWatchIdentifier()
{
    QString identifier("88888888");
    QString accountId("mock/ofono/account0");

    ContactMatcher::instance()->watchIdentifier(accountId, identifier);

    // now add a contact and make sure we get the contactInfoChanged signal
    QSignalSpy contactInfoSpy(ContactMatcher::instance(), SIGNAL(contactInfoChanged(QString,QString,QVariantMap)));
    QContact contact = createContact("Contact", "Watched", QStringList() << identifier);
    QTRY_COMPARE(contactInfoSpy.count(), 1);
    QCOMPARE(contactInfoSpy.first()[0].toString(), accountId);
    QCOMPARE(contactInfoSpy.first()[1].toString(), identifier);
    QVariantMap info = contactInfoSpy.first()[2].toMap();
    QCOMPARE(info[History::FieldContactId].toString(), contact.id().toString());

    QVERIFY(mContactManager->removeContact(contact.id()));
}

QContact ContactMatcherTest::createContact(const QString &firstName, const QString &lastName, const QStringList &phoneNumbers, const QStringList &extendedDetails)
{
    QContact contact;

    QContactName name;
    name.setFirstName(firstName);
    name.setLastName(lastName);

    if (!contact.saveDetail(&name)) {
        return contact;
    }

    Q_FOREACH(const QString &number, phoneNumbers) {
        QContactPhoneNumber phoneNumber;
        phoneNumber.setNumber(number);
        if (!contact.saveDetail(&phoneNumber)) {
            return contact;
        }
    }

    Q_FOREACH(const QString &extended, extendedDetails) {
        QContactExtendedDetail extendedDetail;
        extendedDetail.setName("x-mock-im");
        extendedDetail.setData(extended);
        if (!contact.saveDetail(&extendedDetail)) {
            return contact;
        }
    }

    mContactManager->saveContact(&contact);
    return contact;
}

QTEST_MAIN(ContactMatcherTest)
#include "ContactMatcherTest.moc"
