/*
 * Copyright (C) 2014-2015 Canonical, Ltd.
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
#include "telepathyhelper_p.h"
#include "types.h"
#include "utils_p.h"
#include <QContact>
#include <QContactAvatar>
#include <QContactDisplayLabel>
#include <QContactFilter>
#include <QContactIntersectionFilter>
#include <QContactUnionFilter>
#include <QContactDetailFilter>
#include <QContactExtendedDetail>
#include <QContactPhoneNumber>

using namespace QtContacts;

ContactMatcher::ContactMatcher(QContactManager *manager, QObject *parent) :
    QObject(parent), mManager(manager)
{
    if (!mManager) {
        mManager = new QContactManager("galera");
    }

    // just trigger the creation of TelepathyHelper
    connect(History::TelepathyHelper::instance(), SIGNAL(setupReady()), SLOT(onSetupReady()));

    connect(mManager,
            SIGNAL(contactsAdded(QList<QContactId>)),
            SLOT(onContactsAdded(QList<QContactId>)));
    connect(mManager,
            SIGNAL(contactsChanged(QList<QContactId>)),
            SLOT(onContactsChanged(QList<QContactId>)));
    connect(mManager,
            SIGNAL(contactsRemoved(QList<QContactId>)),
            SLOT(onContactsRemoved(QList<QContactId>)));
    connect(mManager,
            SIGNAL(dataChanged()),
            SLOT(onDataChanged()));
}

void ContactMatcher::onSetupReady()
{
    Q_FOREACH(const RequestInfo &request, mPendingRequests) {
        requestContactInfo(request.accountId, request.identifier);
    }
    mPendingRequests.clear();
}

ContactMatcher::~ContactMatcher()
{
    Q_FOREACH(QContactFetchRequest *request, mRequests.keys()) {
        request->deleteLater();
    }
    mRequests.clear();
    mContactMap.clear();
    mManager->deleteLater();
}

ContactMatcher *ContactMatcher::instance(QContactManager *manager)
{
    static ContactMatcher self(manager);
    return &self;
}

/**
 * \brief Returns the contact information for the given \param identifier, taking into account
 * the addressable fields of the given \param accountId.
 * If \param synchronous is specified, a blocking synchronous request will be made to the contact
 * manager to return the specified data.
 *
 * Note that synchronous requests should only be placed after \ref TelepathyHelper is ready.
 */
QVariantMap ContactMatcher::contactInfo(const QString &accountId, const QString &identifier, bool synchronous)
{
    InternalContactMap &internalMap = mContactMap[accountId];

    // first do a simple string match on the map
    if (internalMap.contains(identifier)) {
        return internalMap[identifier];
    }

    QVariantMap map;
    // and if there was no match, asynchronously request the info, and return an empty map for now
    if (History::TelepathyHelper::instance()->ready()) {
        map = requestContactInfo(accountId, identifier, synchronous);
    } else if (!synchronous) {
        RequestInfo info{accountId, identifier};
        mPendingRequests.append(info);
    }
    map[History::FieldIdentifier] = identifier;
    map[History::FieldAccountId] = accountId;
    mContactMap[accountId][identifier] = map;
    return map;
}

QVariantList ContactMatcher::contactInfo(const QString &accountId, const QStringList &identifiers, bool synchronous)
{
    QVariantList contacts;
    Q_FOREACH(const QString &identifier, identifiers) {
        contacts << contactInfo(accountId, identifier, synchronous);
    }
    return contacts;
}

void ContactMatcher::watchIdentifier(const QString &accountId, const QString &identifier, const QVariantMap &currentInfo)
{
    // only add the identifier to the map of watched identifiers
    QVariantMap map = currentInfo;
    map[History::FieldIdentifier] = identifier;
    mContactMap[accountId][identifier] = map;
}

void ContactMatcher::onContactsAdded(QList<QContactId> ids)
{
    QList<QContact> contacts = mManager->contacts(ids);

    // walk through the list of requested phone numbers
    ContactMap::iterator it = mContactMap.begin();
    ContactMap::iterator end = mContactMap.end();
    for (; it != end; ++it) {
        QString accountId = it.key();

        InternalContactMap &internalMap = it.value();
        InternalContactMap::iterator it2 = internalMap.begin();
        InternalContactMap::iterator end2 = internalMap.end();
        for (; it2 != end2; ++it2) {
            QString identifier = it2.key();
            // skip entries that already have a match
            if (hasMatch(it2.value())) {
                continue;
            }

            // now for each entry not populated, check if it matches one of the newly added contacts
            Q_FOREACH(const QContact &contact, contacts) {
                QVariantMap map = matchAndUpdate(accountId, identifier, contact);
                if (hasMatch(map)){
                    break;
                }
            }
        }
    }
}

void ContactMatcher::onContactsChanged(QList<QContactId> ids)
{
    QList<QContact> contacts = mManager->contacts(ids);

    // walk through the list of requested phone numbers
    ContactMap::iterator it = mContactMap.begin();
    ContactMap::iterator end = mContactMap.end();
    for (; it != end; ++it) {
        QString accountId = it.key();

        InternalContactMap &internalMap = it.value();
        InternalContactMap::iterator it2 = internalMap.begin();
        InternalContactMap::iterator end2 = internalMap.end();
        QStringList identifiersToMatch;

        for (; it2 != end2; ++it2) {
            QVariantMap &contactInfo = it2.value();
            QString identifier = it2.key();

            Q_FOREACH(const QContact &contact, contacts) {
                bool previousMatch = (contactInfo[History::FieldContactId].toString() == contact.id().toString());
                QVariantMap map = matchAndUpdate(accountId, identifier, contact);
                if (hasMatch(map)){
                    break;
                } else if (previousMatch) {
                    // if there was a previous match but it does not match anymore, try to match the phone number
                    // to a different contact
                    identifiersToMatch << identifier;
                    break;
                }
            }
        }

        Q_FOREACH(const QString &identifier, identifiersToMatch) {
            internalMap.remove(identifier);
            requestContactInfo(accountId, identifier);
        }
    }
}

void ContactMatcher::onContactsRemoved(QList<QContactId> ids)
{
    // search for entries that were matching this  contact
    ContactMap::iterator it = mContactMap.begin();
    ContactMap::iterator end = mContactMap.end();
    for (; it != end; ++it) {
        QString accountId = it.key();
        InternalContactMap &internalMap = it.value();
        InternalContactMap::iterator it2 = internalMap.begin();
        InternalContactMap::iterator end2 = internalMap.end();

        QStringList identifiersToMatch;

        for (; it2 != end2; ++it2) {
            // skip entries that didn't have a match
            if (!it2.value().contains(History::FieldContactId)) {
                continue;
            }

            Q_FOREACH(const QContactId &id, ids) {
                if (id.toString() == it2.value()[History::FieldContactId].toString()) {
                    identifiersToMatch << it2.key();
                    break;
                }
            }
        }

        // now make sure to try a new match on the phone numbers whose contact was removed
        Q_FOREACH(const QString &identifier, identifiersToMatch) {
            internalMap.remove(identifier);
            Q_EMIT contactInfoChanged(accountId, identifier, contactInfo(accountId, identifier));
        }
    }
}

void ContactMatcher::onDataChanged()
{
    ContactMap::iterator it = mContactMap.begin();
    ContactMap::iterator end = mContactMap.end();

    for (; it != end; ++it) {
        QString accountId = it.key();
        InternalContactMap &internalMap = it.value();

        // invalidate the cache
        QStringList identifiers = internalMap.keys();
        internalMap.clear();

        Q_FOREACH(const QString &identifier, identifiers) {
            QVariantMap info;
            info[History::FieldIdentifier] = identifier;
            Q_EMIT contactInfoChanged(accountId, identifier, info);
            requestContactInfo(accountId, identifier);
        }
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

        RequestInfo info = mRequests.take(request);
        QContact contact;
        if (!request->contacts().isEmpty()) {
            contact = request->contacts().first();
        }
        matchAndUpdate(info.accountId, info.identifier, contact);
    } else if (state == QContactAbstractRequest::CanceledState) {
        request->deleteLater();
        mRequests.remove(request);
    }

}

/**
 * \brief Requests contact info, and if the preference is for a synchronous request returns the contact information in a
 * QVariantMap. For asynchronous requests, an empty QVariantMap is returned.
 */
QVariantMap ContactMatcher::requestContactInfo(const QString &accountId, const QString &identifier, bool synchronous)
{
    QStringList addressableVCardFields = addressableFields(accountId);
    if (addressableVCardFields.isEmpty()) {
        // FIXME: add support for generic accounts
        return QVariantMap();
    }

    bool phoneCompare = addressableVCardFields.contains("tel");

    QContactFetchHint hint;
    hint.setMaxCountHint(1);
    // FIXME: maybe we need to fetch the full contact?
    hint.setDetailTypesHint(QList<QContactDetail::DetailType>() << QContactDetail::TypeDisplayLabel
                                                                << QContactDetail::TypePhoneNumber
                                                                << QContactDetail::TypeAvatar
                                                                << QContactDetail::TypeExtendedDetail);

    QContactUnionFilter topLevelFilter;
    Q_FOREACH(const QString &field, addressableVCardFields) {
        if (field == "tel") {
            topLevelFilter.append(QContactPhoneNumber::match(identifier));
        } else {
            // FIXME: handle more fields
            // rely on a generic field filter
            QContactDetailFilter nameFilter = QContactDetailFilter();
            nameFilter.setDetailType(QContactExtendedDetail::Type, QContactExtendedDetail::FieldName);
            nameFilter.setMatchFlags(QContactFilter::MatchExactly);
            nameFilter.setValue(field);

            QContactDetailFilter valueFilter = QContactDetailFilter();
            valueFilter.setDetailType(QContactExtendedDetail::Type, QContactExtendedDetail::FieldData);
            valueFilter.setMatchFlags(QContactFilter::MatchExactly);
            valueFilter.setValue(identifier);

            QContactIntersectionFilter intersectionFilter;
            intersectionFilter.append(nameFilter);
            intersectionFilter.append(valueFilter);

            topLevelFilter.append(intersectionFilter);
        }
    }

    if (synchronous) {
        QList<QContact> contacts = mManager->contacts(topLevelFilter, QList<QContactSortOrder>(), hint);
        if (contacts.isEmpty()) {
            return QVariantMap();
        }
        // for synchronous requests, return the results right away.
        return matchAndUpdate(accountId, identifier, contacts.first());
    } else {
        // check if there is a request already going on for the given contact
        Q_FOREACH(const RequestInfo &info, mRequests.values()) {
            if (info.accountId != accountId) {
                // skip to the next item
                continue;
            }

            if (info.identifier == identifier) {
                // if so, just wait for it to finish
                return QVariantMap();
            }
        }

        QContactFetchRequest *request = new QContactFetchRequest(this);
        request->setFetchHint(hint);
        request->setFilter(topLevelFilter);
        request->setManager(mManager);
        connect(request,
                SIGNAL(stateChanged(QContactAbstractRequest::State)),
                SLOT(onRequestStateChanged(QContactAbstractRequest::State)));

        RequestInfo info;
        info.accountId = accountId;
        info.identifier = identifier;
        mRequests[request] = info;
        request->start();
    }
    return QVariantMap();
}

QVariantList ContactMatcher::toVariantList(const QList<int> &list)
{
    QVariantList variantList;
    Q_FOREACH(int value, list) {
        variantList << value;
    }
    return variantList;
}

/**
 * \brief Matches contact data against the given identifier. If the match succeeds, return the updated data in a
 * QVariantMap, returns an empty map otherwise.
 */
QVariantMap ContactMatcher::matchAndUpdate(const QString &accountId, const QString &identifier, const QContact &contact)
{
    QVariantMap contactInfo;
    contactInfo[History::FieldIdentifier] = identifier;
    contactInfo[History::FieldAccountId] = accountId;

    if (contact.isEmpty()) {
        return contactInfo;
    }

    QStringList fields = addressableFields(accountId);
    bool match = false;

    int fieldsCount = fields.count();
    Q_FOREACH(const QString &field, fields) {
        if (field == "tel") {
            QList<QContactDetail> details = contact.details(QContactDetail::TypePhoneNumber);
            Q_FOREACH(const QContactPhoneNumber number, details) {
                if (PhoneUtils::comparePhoneNumbers(number.number(), identifier)) {
                    QVariantMap detailProperties;
                    detailProperties["phoneSubTypes"] = toVariantList(number.subTypes());
                    detailProperties["phoneContexts"] = toVariantList(number.contexts());
                    contactInfo[History::FieldDetailProperties] = detailProperties;
                    match = true;
                    break;
                }
            }
        } else {
            // FIXME: support more types of field
            // generic code for extra fields
            Q_FOREACH(const QContactExtendedDetail detail, contact.details(QContactDetail::TypeExtendedDetail)) {
                if (detail.name() == field && detail.data() == identifier) {
                    match = true;
                    break;
                }
            }
        }

        if (match) {
            break;
        }
    }

    if (match) {
        contactInfo[History::FieldContactId] = contact.id().toString();
        contactInfo[History::FieldAlias] = QContactDisplayLabel(contact.detail(QContactDetail::TypeDisplayLabel)).label();
        contactInfo[History::FieldAvatar] = QContactAvatar(contact.detail(QContactDetail::TypeAvatar)).imageUrl().toString();

        mContactMap[accountId][identifier] = contactInfo;
        Q_EMIT contactInfoChanged(accountId, identifier, contactInfo);
    }


    return contactInfo;
}

QStringList ContactMatcher::addressableFields(const QString &accountId)
{
    if (mAddressableFields.contains(accountId)) {
        return mAddressableFields[accountId];
    }

    Tp::AccountPtr account = History::TelepathyHelper::instance()->accountForId(accountId);
    QStringList fields;
    if (!account.isNull()) {
        fields = account->protocolInfo().addressableVCardFields();
        mAddressableFields[accountId] = fields;
    }

    return fields;
}

bool ContactMatcher::hasMatch(const QVariantMap &map) const
{
    return !map[History::FieldContactId].toString().isEmpty();
}
