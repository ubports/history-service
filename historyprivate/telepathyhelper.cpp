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

#include "telepathyhelper_p.h"

#include <TelepathyQt/ClientRegistrar>

TelepathyHelper::TelepathyHelper(QObject *parent)
    : QObject(parent),
      mChannelObserver(0)
{
    mAccountFeatures << Tp::Account::FeatureCore
                     << Tp::Account::FeatureProtocolInfo;
    mContactFeatures << Tp::Contact::FeatureAlias
                     << Tp::Contact::FeatureAvatarData
                     << Tp::Contact::FeatureAvatarToken
                     << Tp::Contact::FeatureCapabilities
                     << Tp::Contact::FeatureSimplePresence;
    mConnectionFeatures << Tp::Connection::FeatureCore
                        << Tp::Connection::FeatureSelfContact
                        << Tp::Connection::FeatureSimplePresence;

    Tp::ChannelFactoryPtr channelFactory = Tp::ChannelFactory::create(QDBusConnection::sessionBus());
    channelFactory->addCommonFeatures(Tp::Channel::FeatureCore);

    mAccountManager = Tp::AccountManager::create(
            Tp::AccountFactory::create(QDBusConnection::sessionBus(), mAccountFeatures),
            Tp::ConnectionFactory::create(QDBusConnection::sessionBus(), mConnectionFeatures),
            channelFactory,
            Tp::ContactFactory::create(mContactFeatures));

    connect(mAccountManager->becomeReady(Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));

    mClientRegistrar = Tp::ClientRegistrar::create(mAccountManager);
}

TelepathyHelper::~TelepathyHelper()
{
}

TelepathyHelper *TelepathyHelper::instance()
{
    static TelepathyHelper* helper = new TelepathyHelper();
    return helper;
}

ChannelObserver *TelepathyHelper::channelObserver() const
{
    return mChannelObserver;
}

void TelepathyHelper::registerChannelObserver()
{
    // check if this instance is running on the main phone application
    // or if it is just the plugin imported somewhere else
    QString observerName = "HistoryDaemonObserver";

    mChannelObserver = new ChannelObserver(this);
    registerClient(mChannelObserver, observerName);
    Q_EMIT channelObserverCreated(mChannelObserver);
}

void TelepathyHelper::registerClient(Tp::AbstractClient *client, QString name)
{
    Tp::AbstractClientPtr clientPtr(client);
    bool succeeded = mClientRegistrar->registerClient(clientPtr, name);
    if (!succeeded) {
        name.append("%1");
        int count = 0;
        // limit the number of registered clients to 20, that should be a safe margin
        while (!succeeded && count < 20) {
            succeeded = mClientRegistrar->registerClient(clientPtr, name.arg(++count));
            if (succeeded) {
                name = name.arg(count);
            }
        }
    }

    if (succeeded) {
        QObject *object = dynamic_cast<QObject*>(client);
        if (object) {
            object->setProperty("clientName", TP_QT_IFACE_CLIENT + "." + name );
        }
    }
}

Tp::AccountPtr TelepathyHelper::accountForId(const QString &accountId)
{
    Q_FOREACH(const Tp::AccountPtr &account, mAccounts) {
        if (account->uniqueIdentifier() == accountId) {
            return account;
        }
    }

    return Tp::AccountPtr();
}

QList<Tp::AccountPtr> TelepathyHelper::accounts() const
{
    return mAccounts;
}

void TelepathyHelper::onAccountManagerReady(Tp::PendingOperation *op)
{
    Q_UNUSED(op)

    Q_FOREACH(const Tp::AccountPtr &account, mAccountManager->allAccounts()) {
        onNewAccount(account);
    }

    connect(mAccountManager.data(),
            SIGNAL(newAccount(Tp::AccountPtr)),
            SLOT(onNewAccount(Tp::AccountPtr)));

    Q_EMIT setupReady();
}

void TelepathyHelper::onNewAccount(const Tp::AccountPtr &account)
{
    connect(account.data(),
            SIGNAL(removed()),
            SLOT(onAccountRemoved()));
    mAccounts.append(account);
    Q_EMIT accountAdded(account);
}

void TelepathyHelper::onAccountRemoved()
{
    Tp::AccountPtr account(qobject_cast<Tp::Account*>(sender()));
    if (account.isNull() || !mAccounts.contains(account)) {
        qWarning() << "The removed account was not found.";
        return;
    }

    account.data()->disconnect(this);
    mAccounts.removeAll(account);
    Q_EMIT accountRemoved(account);
}


bool TelepathyHelper::connected() const
{
    Q_FOREACH(const Tp::AccountPtr &account, mAccounts) {
        if (!account->connection().isNull()) {
            return true;
        }
    }

    return false;
}
