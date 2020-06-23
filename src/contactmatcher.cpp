/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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

namespace History
{

ContactMatcher::ContactMatcher(QContactManager *manager, QObject *parent) :
    QObject(parent), mManager(manager)
{
    if (!mManager) {
        mManager = new QContactManager("galera");
    }

    // just trigger the creation of TelepathyHelper
    connect(History::TelepathyHelper::instance(), SIGNAL(setupReady()), SLOT(onSetupReady()));

    QObject::connect(mManager, &QContactManager::contactsAdded,
                     this, &ContactMatcher::onContactsAdded);
    QObject::connect(mManager, &QContactManager::contactsChanged,
                     this, &ContactMatcher::onContactsChanged);
    QObject::connect(mManager, &QContactManager::contactsRemoved,
                     this, &ContactMatcher::onContactsRemoved);
    QObject::connect(mManager, &QContactManager::dataChanged,
                     this, &ContactMatcher::onDataChanged);
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
QVariantMap ContactMatcher::contactInfo(const QString &accountId, const QString &identifier, bool synchronous, const QVariantMap &properties)
{
    InternalContactMap &internalMap = mContactMap[accountId];


    QString normalizedId = normalizeId(identifier);

    QVariantMap map;
    // first do a simple string match on the map
    if (internalMap.contains(normalizedId)) {
        map = internalMap[normalizedId];
    } else if (History::TelepathyHelper::instance()->ready()) {
        // and if there was no match, asynchronously request the info, and return an empty map for now
        map = requestContactInfo(accountId, normalizedId, synchronous);
    } else if (!synchronous) {
        RequestInfo info{accountId, normalizedId};
        mPendingRequests.append(info);
    }

    map[History::FieldIdentifier] = normalizedId;
    map[History::FieldAccountId] = accountId;

    QMapIterator<QString, QVariant> i(properties);
    while (i.hasNext()) {
        i.next();
        if (!map.contains(i.key())) {
            map[i.key()] = i.value();
        }
    }

    mContactMap[accountId][normalizedId] = map;
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
                bool previousMatch = (contactInfo.contains(History::FieldContactId) &&
                                      contactInfo[History::FieldContactId].toString() == contact.id().toString());
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
            QVariantMap &info = it2.value();
            // skip entries that didn't have a match
            if (!hasMatch(info)) {
                continue;
            }

            Q_FOREACH(const QContactId &id, ids) {
                if (id.toString() == info[History::FieldContactId].toString()) {
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
    QString normalizedId = normalizeId(identifier);
    QStringList addressableVCardFields = addressableFields(accountId);

    QVariantMap contactInfo;
    contactInfo[History::FieldIdentifier] = identifier;
    contactInfo[History::FieldAccountId] = accountId;

    if (addressableVCardFields.isEmpty()) {
        mContactMap[accountId][identifier] = contactInfo;
        // FIXME: add support for generic accounts
        return contactInfo;
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
            topLevelFilter.append(QContactPhoneNumber::match(normalizedId));
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
            valueFilter.setValue(normalizedId);

            QContactIntersectionFilter intersectionFilter;
            intersectionFilter.append(nameFilter);
            intersectionFilter.append(valueFilter);

            topLevelFilter.append(intersectionFilter);
        }
    }

    if (synchronous) {
        QList<QContact> contacts = mManager->contacts(topLevelFilter, QList<QContactSortOrder>(), hint);
        if (contacts.isEmpty()) {
            mContactMap[accountId][identifier] = contactInfo;
            return contactInfo;
        }
        // for synchronous requests, return the results right away.
        return matchAndUpdate(accountId, normalizedId, contacts.first());
    } else {
        // check if there is a request already going on for the given contact
        Q_FOREACH(const RequestInfo &info, mRequests.values()) {
            if (info.accountId != accountId) {
                // skip to the next item
                continue;
            }

            if (info.identifier == normalizedId) {
                // if so, just wait for it to finish
                return QVariantMap();
            }
        }

        QContactFetchRequest *request = new QContactFetchRequest(this);
        request->setFetchHint(hint);
        request->setFilter(topLevelFilter);
        request->setManager(mManager);
        QObject::connect(request, &QContactFetchRequest::stateChanged,
                         this, &ContactMatcher::onRequestStateChanged);

        RequestInfo info;
        info.accountId = accountId;
        info.identifier = normalizedId;
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

    Q_FOREACH(const QString &field, fields) {
        if (field == "tel") {
            QList<QContactDetail> details = contact.details(QContactDetail::TypePhoneNumber);
            Q_FOREACH(const QContactPhoneNumber number, details) {
                if (History::PhoneUtils::comparePhoneNumbers(number.number(), identifier)) {
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

    // FIXME: hardcoding account IDs here is not a good idea, we have to fix addressable fields on
    // the protocols themselves
    if (accountId.startsWith("irc/irc")) {
        QStringList empty;
        mAddressableFields[accountId] = empty;
        return empty;
    }

    Tp::AccountPtr account = History::TelepathyHelper::instance()->accountForId(accountId);
    QStringList fields;
    if (!account.isNull()) {
        fields = account->protocolInfo().addressableVCardFields();
    }

    // fallback to phone number matching in case everything else fails
    if (fields.isEmpty()) {
        fields << "tel";
    }

    mAddressableFields[accountId] = fields;

    return fields;
}

bool ContactMatcher::hasMatch(const QVariantMap &map) const
{
    return (map.contains(History::FieldContactId) && !map[History::FieldContactId].toString().isEmpty());
}

QString ContactMatcher::normalizeId(const QString &id)
{
    QString normalizedId = id;

    // FIXME: this is a hack so that SIP URIs get converted into phone numbers for contact matching
    if (normalizedId.startsWith("sip:")) {
        normalizedId.remove("sip:").remove(QRegularExpression("@.*$"));
    }

    return normalizedId;
}


}
