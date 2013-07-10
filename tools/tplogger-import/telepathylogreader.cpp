/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of phone-app.
 *
 * phone-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * phone-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "telepathylogreader.h"
#include <TelepathyLoggerQt/LogManager>
#include <TelepathyLoggerQt/PendingDates>
#include <TelepathyLoggerQt/PendingEntities>
#include <TelepathyLoggerQt/PendingEvents>
#include <TelepathyLoggerQt/Entity>
#include <TelepathyLoggerQt/Event>
#include <TelepathyLoggerQt/CallEvent>
#include <TelepathyLoggerQt/TextEvent>
#include <TelepathyQt/PendingReady>

TelepathyLogReader::TelepathyLogReader(QObject *parent) :
    QObject(parent), mLogManager(Tpl::LogManager::instance())
{
    Tp::Features accountFeatures;
    Tp::Features contactFeatures;
    accountFeatures << Tp::Account::FeatureCore;
    contactFeatures << Tp::Contact::FeatureAlias
                    << Tp::Contact::FeatureAvatarData
                    << Tp::Contact::FeatureAvatarToken
                    << Tp::Contact::FeatureCapabilities
                    << Tp::Contact::FeatureSimplePresence;

    mAccountManager = Tp::AccountManager::create(
                Tp::AccountFactory::create(QDBusConnection::sessionBus(), accountFeatures),
                Tp::ConnectionFactory::create(QDBusConnection::sessionBus()),
                Tp::ChannelFactory::create(QDBusConnection::sessionBus()),
                Tp::ContactFactory::create(contactFeatures));

    connect(mAccountManager->becomeReady(Tp::AccountManager::FeatureCore),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onAccountManagerReady(Tp::PendingOperation*)));
}

TelepathyLogReader *TelepathyLogReader::instance()
{
    static TelepathyLogReader *self = new TelepathyLogReader();
    return self;
}

void TelepathyLogReader::fetchLog(const Tp::AccountPtr &account)
{
    Tpl::PendingEntities *pendingEntities = mLogManager->queryEntities(account);

    /* Fetching the log work like this:
       - Start by fetching the entities from the log
       - Once you get the entities, fetch the available dates
       - After you get the dates, fetch the events themselves
     */

    connect(pendingEntities,
            SIGNAL(finished(Tpl::PendingOperation*)),
            SLOT(onPendingEntitiesFinished(Tpl::PendingOperation*)));
}

void TelepathyLogReader::requestDatesForEntities(const Tp::AccountPtr &account, const Tpl::EntityPtrList &entities)
{
    Q_FOREACH(Tpl::EntityPtr entity, entities) {
        Tpl::PendingDates *pendingDates = mLogManager->queryDates(account, entity, Tpl::EventTypeMaskAny);

        connect(pendingDates,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingDatesFinished(Tpl::PendingOperation*)));
    }
}

void TelepathyLogReader::requestEventsForDates(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity, const Tpl::QDateList &dates)
{
    Q_FOREACH(QDate date, dates) {
        Tpl::PendingEvents *pendingEvents = mLogManager->queryEvents(account, entity, Tpl::EventTypeMaskAny, date);
        connect(pendingEvents,
                SIGNAL(finished(Tpl::PendingOperation*)),
                SLOT(onPendingEventsFinished(Tpl::PendingOperation*)));
    }
}

void TelepathyLogReader::onPendingEntitiesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEntities *pe = qobject_cast<Tpl::PendingEntities*>(op);
    if (!pe) {
        return;
    }

    // request the dates for all the entities
    requestDatesForEntities(pe->account(), pe->entities());
}

void TelepathyLogReader::onPendingDatesFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingDates *pd = qobject_cast<Tpl::PendingDates*>(op);
    if (!pd) {
        return;
    }

    // request all events
    requestEventsForDates(pd->account(), pd->entity(), pd->dates());
}

void TelepathyLogReader::onPendingEventsFinished(Tpl::PendingOperation *op)
{
    Tpl::PendingEvents *pe = qobject_cast<Tpl::PendingEvents*>(op);
    if (!pe) {
        return;
    }

    Q_FOREACH(const Tpl::EventPtr &event, pe->events()) {
        bool incoming = event->receiver()->entityType() == Tpl::EntityTypeSelf;
        Tpl::EntityPtr remoteEntity = incoming ? event->sender() : event->receiver();

        QString phoneNumber = remoteEntity->identifier();
        QDateTime timestamp = event->timestamp();

        Tpl::CallEventPtr callEvent = event.dynamicCast<Tpl::CallEvent>();
        Tpl::TextEventPtr textEvent = event.dynamicCast<Tpl::TextEvent>();

        if (!callEvent.isNull()) {
            Q_EMIT loadedCallEvent(callEvent);
        }

        if (!textEvent.isNull()) {
            Q_EMIT loadedMessageEvent(textEvent);
        }
    }
}


void TelepathyLogReader::onAccountManagerReady(Tp::PendingOperation *op)
{
    mLogManager->setAccountManagerPtr(mAccountManager);
    Q_FOREACH(const Tp::AccountPtr account, mAccountManager->allAccounts()) {
        fetchLog(account);
    }
}
