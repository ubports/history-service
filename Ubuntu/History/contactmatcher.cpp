/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#include "contactmatcher_p.h"
#include "phoneutils_p.h"
#include "types.h"
#include <QContact>
#include <QContactAvatar>
#include <QContactDisplayLabel>
#include <QContactFilter>
#include <QContactPhoneNumber>

using namespace QtContacts;

ContactMatcher::ContactMatcher(QObject *parent) :
    QObject(parent), mManager(new QContactManager("galera"))
{
    connect(mManager,
            SIGNAL(contactsAdded(QList<QContactId>)),
            SLOT(onContactsAdded(QList<QContactId>)));
    connect(mManager,
            SIGNAL(contactsChanged(QList<QContactId>)),
            SLOT(onContactsChanged(QList<QContactId>)));
    connect(mManager,
            SIGNAL(contactsRemoved(QList<QContactId>)),
            SLOT(onContactsRemoved(QList<QContactId>)));
}

ContactMatcher *ContactMatcher::instance()
{
    static ContactMatcher *self = new ContactMatcher();
    return self;
}

QVariantMap ContactMatcher::contactInfo(const QString &phoneNumber)
{
    // first do a simple stirng match on the map
    if (mContactMap.contains(phoneNumber)) {
        return mContactMap[phoneNumber];
    }

    // now if there was no string match, try phone number matching
    Q_FOREACH(const QString &key, mContactMap.keys()) {
        if (PhoneUtils::comparePhoneNumbers(key, phoneNumber)) {
            return mContactMap[key];
        }
    }

    // and if there was no match, asynchronously request the info, and return an empty map for now
    requestContactInfo(phoneNumber);
    QVariantMap map;
    map[History::FieldPhoneNumber] = phoneNumber;
    return map;
}

QVariantList ContactMatcher::contactInfo(const QStringList &numbers)
{
    QVariantList contacts;
    Q_FOREACH(const QString &number, numbers) {
        contacts << contactInfo(number);
    }
    return contacts;
}

void ContactMatcher::onContactsAdded(QList<QContactId> ids)
{
    QList<QContact> contacts = mManager->contacts(ids);

    // walk through the list of requested phone numbers
    ContactMap::iterator it = mContactMap.begin();
    ContactMap::iterator end = mContactMap.end();
    for (; it != end; ++it) {
        QString phoneNumber = it.key();
        // skip entries that already have a match
        if (it.value().contains(History::FieldContactId)) {
            continue;
        }

        // now for each entry not populated, check if it matches one of the newly added contacts
        bool found = false;
        Q_FOREACH(const QContact &contact, contacts) {
            Q_FOREACH(const QContactPhoneNumber number, contact.details(QContactDetail::TypePhoneNumber)) {
                if (PhoneUtils::comparePhoneNumbers(number.number(), phoneNumber)) {
                    found = true;
                    populateInfo(phoneNumber, contact);
                    break;
                }
            }

            if (found) {
                break;
            }
        }
    }
}

void ContactMatcher::onContactsChanged(QList<QContactId> ids)
{
    QStringList phoneNumbersToMatch;

    QList<QContact> contacts = mManager->contacts(ids);
    // walk through the list of requested phone numbers
    ContactMap::iterator it = mContactMap.begin();
    ContactMap::iterator end = mContactMap.end();
    for (; it != end; ++it) {
        QVariantMap &contactInfo = it.value();
        QString phoneNumber = it.key();

        Q_FOREACH(const QContact &contact, contacts) {
            bool previousMatch = (contactInfo[History::FieldContactId].toString() == contact.id().toString());
            bool found = false;
            Q_FOREACH(const QContactPhoneNumber number, contact.details(QContactDetail::TypePhoneNumber)) {
                if (PhoneUtils::comparePhoneNumbers(number.number(), phoneNumber)) {
                    found = true;
                    break;
                }
            }

            if (found) {
                populateInfo(phoneNumber, contact);
                break;
            } else if (previousMatch) {
                // if there was a previous match but it does not match anymore, try to match the phone number
                // to a different contact
                phoneNumbersToMatch << phoneNumber;
                break;
            }
        }
    }

    Q_FOREACH(const QString &phoneNumber, phoneNumbersToMatch) {
        mContactMap.remove(phoneNumber);
        requestContactInfo(phoneNumber);
    }
}

void ContactMatcher::onContactsRemoved(QList<QContactId> ids)
{
    QStringList phoneNumbersToMatch;

    // search for entries that were matching this  contact
    ContactMap::iterator it = mContactMap.begin();
    ContactMap::iterator end = mContactMap.end();
    for (; it != end; ++it) {
        // skip entries that didn't have a match
        if (!it.value().contains(History::FieldContactId)) {
            continue;
        }

        Q_FOREACH(const QContactId &id, ids) {
            if (id.toString() == it.value()[History::FieldContactId].toString()) {
                phoneNumbersToMatch << it.key();
                break;
            }
        }
    }

    // now make sure to try a new match on the phone numbers whose contact was removed
    Q_FOREACH(const QString &phoneNumber, phoneNumbersToMatch) {
        mContactMap.remove(phoneNumber);
        requestContactInfo(phoneNumber);
    }
}

void ContactMatcher::onRequestStateChanged(QContactAbstractRequest::State state)
{
    QContactFetchRequest *request = qobject_cast<QContactFetchRequest*>(sender());
    if (!request) {
        return;
    }

    if (!mRequests.contains(request)) {
        request->deleteLater();
        return;
    }

    if (state == QContactAbstractRequest::FinishedState) {
        request->deleteLater();

        QString phoneNumber = mRequests.take(request);
        QContact contact;
        if (!request->contacts().isEmpty()) {
            contact = request->contacts().first();
        }
        populateInfo(phoneNumber, contact);
    } else if (state == QContactAbstractRequest::CanceledState) {
        request->deleteLater();
        mRequests.remove(request);
    }

}

void ContactMatcher::requestContactInfo(const QString &phoneNumber)
{
    // check if there is a request already going on for the given contact
    Q_FOREACH(const QString number, mRequests.values()) {
        if (PhoneUtils::comparePhoneNumbers(number, phoneNumber)) {
            // if so, just wait for it to finish
            return;
        }
    }

    QContactFetchRequest *request = new QContactFetchRequest(this);
    mRequests[request] = phoneNumber;
    request->setFilter(QContactPhoneNumber::match(phoneNumber));
    connect(request, SIGNAL(stateChanged(QContactAbstractRequest::State)), SLOT(onRequestStateChanged(QContactAbstractRequest::State)));
    request->setManager(mManager);
    request->start();
}

QVariantList ContactMatcher::toVariantList(const QList<int> &list)
{
    QVariantList variantList;
    Q_FOREACH(int value, list) {
        variantList << value;
    }
    return variantList;
}

void ContactMatcher::populateInfo(const QString &phoneNumber, const QContact &contact)
{
    QVariantMap contactInfo;
    contactInfo[History::FieldPhoneNumber] = phoneNumber;
    if (!contact.isEmpty()) {
        contactInfo[History::FieldContactId] = contact.id().toString();
        contactInfo[History::FieldAlias] = QContactDisplayLabel(contact.detail(QContactDetail::TypeDisplayLabel)).label();
        contactInfo[History::FieldAvatar] = QContactAvatar(contact.detail(QContactDetail::TypeAvatar)).imageUrl().toString();
        //contactInfo[History::FieldAlias] = contact.dis
        Q_FOREACH(const QContactPhoneNumber number, contact.details(QContactDetail::TypePhoneNumber)) {
            if (PhoneUtils::comparePhoneNumbers(number.number(), phoneNumber)) {
                contactInfo[History::FieldPhoneSubTypes] = toVariantList(number.subTypes());
                contactInfo[History::FieldPhoneContexts] = toVariantList(number.contexts());
            }
        }
    }
    mContactMap[phoneNumber] = contactInfo;
    Q_EMIT contactInfoChanged(phoneNumber, contactInfo);
}
