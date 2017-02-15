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

#ifndef CONTACTMATCHER_P_H
#define CONTACTMATCHER_P_H

#include <QObject>
#include <QVariantMap>
#include <QContactFetchRequest>
#include <QContactManager>

using namespace QtContacts;

namespace History
{

typedef QMap<QString, QVariantMap> InternalContactMap;
typedef QMap<QString, InternalContactMap> ContactMap;

typedef struct {
    QString accountId;
    QString identifier;
} RequestInfo;

class ContactMatcher : public QObject
{
    Q_OBJECT
public:
    static ContactMatcher *instance(QContactManager *manager = 0);
    QVariantMap contactInfo(const QString &accountId, const QString &identifier, bool synchronous = false, const QVariantMap &properties = QVariantMap());
    QVariantList contactInfo(const QString &accountId, const QStringList &identifiers, bool synchronous = false);

    // this will only watch for contact changes affecting the identifier, but won't fetch contact info
    void watchIdentifier(const QString &accountId, const QString &identifier, const QVariantMap &currentInfo = QVariantMap());

    static QString normalizeId(const QString &id);

Q_SIGNALS:
    void contactInfoChanged(const QString &acountId, const QString &identifier, const QVariantMap &contactInfo);

protected Q_SLOTS:
    void onContactsAdded(QList<QContactId> ids);
    void onContactsChanged(QList<QContactId> ids);
    void onContactsRemoved(QList<QContactId> ids);
    void onDataChanged();
    void onRequestStateChanged(QContactAbstractRequest::State state);
    void onSetupReady();

protected:
    QVariantMap requestContactInfo(const QString &accountId, const QString &identifier, bool synchronous = false);
    QVariantList toVariantList(const QList<int> &list);
    QVariantMap matchAndUpdate(const QString &accountId, const QString &identifier, const QContact &contact);
    QStringList addressableFields(const QString &accountId);
    bool hasMatch(const QVariantMap &map) const;

private:
    explicit ContactMatcher(QContactManager *manager = 0, QObject *parent = 0);
    ~ContactMatcher();

    ContactMap mContactMap;
    QMap<QContactFetchRequest*, RequestInfo> mRequests;
    QMap<QString, QStringList> mAddressableFields;
    QList<RequestInfo> mPendingRequests;
    QContactManager *mManager;
};

}

#endif // CONTACTMATCHER_P_H
