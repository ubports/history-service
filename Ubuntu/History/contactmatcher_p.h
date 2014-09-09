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

#ifndef CONTACTMATCHER_P_H
#define CONTACTMATCHER_P_H

#include <QObject>
#include <QVariantMap>
#include <QContactFetchRequest>
#include <QContactManager>

using namespace QtContacts;

typedef QMap<QString, QVariantMap> ContactMap;
class ContactMatcher : public QObject
{
    Q_OBJECT
public:
    static ContactMatcher *instance();
    QVariantMap contactInfo(const QString &phoneNumber);
    QVariantList contactInfo(const QStringList &numbers);

Q_SIGNALS:
    void contactInfoChanged(const QString &phoneNumber, const QVariantMap &contactInfo);

protected Q_SLOTS:
    void onContactsAdded(QList<QContactId> ids);
    void onContactsChanged(QList<QContactId> ids);
    void onContactsRemoved(QList<QContactId> ids);
    void onRequestStateChanged(QContactAbstractRequest::State state);

protected:
    void requestContactInfo(const QString &phoneNumber);
    QVariantList toVariantList(const QList<int> &list);
    void populateInfo(const QString &phoneNumber, const QContact &contact);

private:
    explicit ContactMatcher(QObject *parent = 0);

    ContactMap mContactMap;
    QMap<QContactFetchRequest*, QString> mRequests;
    QContactManager *mManager;
};

#endif // CONTACTMATCHER_P_H
