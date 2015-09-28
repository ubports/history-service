/*
 * Copyright (C) 2012-2015 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef TELEPATHYHELPER_H
#define TELEPATHYHELPER_H

#include <QObject>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>
#include <TelepathyQt/Connection>
#include <TelepathyQt/ConnectionManager>
#include <TelepathyQt/Types>
#include "channelobserver_p.h"

class TelepathyHelper : public QObject
{
    Q_OBJECT

public:
    ~TelepathyHelper();

    static TelepathyHelper *instance();
    ChannelObserver *channelObserver() const;

    void registerClient(Tp::AbstractClient *client, QString name);
    Tp::AccountPtr accountForId(const QString &accountId);
    QList<Tp::AccountPtr> accounts() const;
    bool connected() const;
    bool ready() const;

Q_SIGNALS:
    void channelObserverCreated(ChannelObserver *observer);
    void accountAdded(const Tp::AccountPtr &account);
    void accountRemoved(const Tp::AccountPtr &account);
    void setupReady();

public Q_SLOTS:
    void registerChannelObserver();

private Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onNewAccount(const Tp::AccountPtr &account);
    void onAccountRemoved();

private:
    explicit TelepathyHelper(QObject *parent = 0);
    Tp::AccountManagerPtr mAccountManager;
    Tp::Features mAccountManagerFeatures;
    Tp::Features mAccountFeatures;
    Tp::Features mContactFeatures;
    Tp::Features mConnectionFeatures;
    Tp::ClientRegistrarPtr mClientRegistrar;
    ChannelObserver *mChannelObserver;
    QList<Tp::AccountPtr> mAccounts;
    bool mReady;
};

#endif // TELEPATHYHELPER_H
