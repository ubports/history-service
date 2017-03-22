/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#include "participant.h"
#include "participant_p.h"
#include "types.h"

namespace History
{

ParticipantPrivate::ParticipantPrivate()
{

}

ParticipantPrivate::ParticipantPrivate(const QString &theAccountId,
                                       const QString &theIdentifier,
                                       const QString &theContactId,
                                       const QString &theAlias,
                                       const QString &theAvatar,
                                       const uint theState,
                                       const uint theRoles,
                                       const QVariantMap &theDetailProperties) :
    accountId(theAccountId), identifier(theIdentifier), contactId(theContactId),
    alias(theAlias), avatar(theAvatar), state(theState), roles(theRoles), detailProperties(theDetailProperties)
{
}

ParticipantPrivate::~ParticipantPrivate()
{
}

Participant::Participant()
    : d_ptr(new ParticipantPrivate())
{
}

Participant::Participant(const QString &accountId, const QString &identifier, const QString &contactId, const QString &alias, const QString &avatar, uint state, uint roles, const QVariantMap &detailProperties)
    : d_ptr(new ParticipantPrivate(accountId, identifier, contactId, alias, avatar, state, roles, detailProperties))
{
}

Participant::Participant(const Participant &other)
    : d_ptr(new ParticipantPrivate(*other.d_ptr))
{
}

Participant &Participant::operator=(const Participant &other)
{
    if (&other == this) {
        return *this;
    }
    d_ptr = QSharedPointer<ParticipantPrivate>(new ParticipantPrivate(*other.d_ptr));
    return *this;
}

Participant::~Participant()
{
}

QString Participant::accountId() const
{
    Q_D(const Participant);
    return d->accountId;
}

QString Participant::identifier() const
{
    Q_D(const Participant);
    return d->identifier;
}

QString Participant::contactId() const
{
    Q_D(const Participant);
    return d->contactId;
}

QString Participant::alias() const
{
    Q_D(const Participant);
    return d->alias;
}

QString Participant::avatar() const
{
    Q_D(const Participant);
    return d->avatar;
}

uint Participant::state() const
{
    Q_D(const Participant);
    return d->state;
}

uint Participant::roles() const
{
    Q_D(const Participant);
    return d->roles;
}

QVariantMap Participant::detailProperties() const
{
    Q_D(const Participant);
    return d->detailProperties;
}

bool Participant::isNull() const
{
    Q_D(const Participant);
    return d->accountId.isNull() || d->identifier.isNull();
}

bool Participant::operator==(const Participant &other) const
{
    Q_D(const Participant);
    return d->accountId == other.d_ptr->accountId && d->identifier == other.d_ptr->identifier;
}

bool Participant::operator<(const Participant &other) const
{
    Q_D(const Participant);
    QString selfData = d->accountId + d->identifier;
    QString otherData = other.d_ptr->accountId + other.d_ptr->identifier;
    return selfData < otherData;
}

QVariantMap Participant::properties() const
{
    Q_D(const Participant);

    QVariantMap map;
    map[FieldAccountId] = d->accountId;
    map[FieldIdentifier] = d->identifier;
    map[FieldContactId] = d->contactId;
    map[FieldAlias] = d->alias;
    map[FieldAvatar] = d->avatar;
    map[FieldParticipantState] = d->state;
    map[FieldParticipantRoles] = d->roles;
    map[FieldDetailProperties] = d->detailProperties;

    return map;
}

Participant Participant::fromProperties(const QVariantMap &properties)
{
    Participant participant;
    if (properties.isEmpty()) {
        return participant;
    }

    QString accountId = properties[FieldAccountId].toString();
    QString identifier = properties[FieldIdentifier].toString();
    QString contactId = properties[FieldContactId].toString();
    QString alias = properties[FieldAlias].toString();
    QString avatar = properties[FieldAvatar].toString();
    uint state = properties[FieldParticipantState].toUInt();
    uint roles = properties[FieldParticipantRoles].toUInt();
    QVariantMap detailProperties;
    QVariant detailPropertiesVariant = properties[FieldDetailProperties];
    if (detailPropertiesVariant.canConvert<QVariantMap>()) {
        detailProperties = detailPropertiesVariant.toMap();
    } else if (detailPropertiesVariant.canConvert<QDBusArgument>()) {
        detailProperties = qdbus_cast<QVariantMap>(detailPropertiesVariant);
        Q_FOREACH(const QString &key, detailProperties.keys()) {
            QList<QVariant> list = qdbus_cast<QList<QVariant> >(detailProperties[key]);
            detailProperties[key] = QVariant::fromValue(list);
        }
    }

    return Participant(accountId, identifier, contactId, alias, avatar, state, roles, detailProperties);
}

QStringList Participants::identifiers() const
{
    QStringList result;
    Q_FOREACH(const Participant &participant, *this) {
        result << participant.identifier();
    }
    return result;
}

Participants Participants::fromVariant(const QVariant &variant)
{
    Participants participants;
    if (variant.type() == QVariant::StringList) {
        participants = Participants::fromStringList(variant.toStringList());
    } else if (variant.canConvert<QVariantList>()) {
        participants = Participants::fromVariantList(variant.toList());
    } else if (variant.canConvert<QDBusArgument>()) {
        QDBusArgument argument = variant.value<QDBusArgument>();
        argument >> participants;
    }
    return participants;
}

Participants Participants::fromStringList(const QStringList &list)
{
    Participants participants;
    Q_FOREACH(const QString& participantId, list) {
        QVariantMap properties;
        properties[FieldIdentifier] = participantId;
        participants << Participant::fromProperties(properties);
    }
    return participants;
}

Participants Participants::fromVariantList(const QVariantList &list)
{
    Participants participants;
    Q_FOREACH(const QVariant& entry, list) {
        participants << Participant::fromProperties(entry.toMap());
    }
    return participants;
}

Participants Participants::fromVariantMapList(const QList<QVariantMap> &list)
{
    Participants participants;
    Q_FOREACH(const QVariantMap& entry, list) {
        participants << Participant::fromProperties(entry);
    }
    return participants;
}

QVariantList Participants::toVariantList() const
{
    QVariantList list;
    Q_FOREACH(const Participant &participant, *this) {
        list << participant.properties();
    }
    return list;
}

Participants Participants::filterByState(uint state) const
{
    Participants filtered;
    Q_FOREACH(const Participant &participant, *this) {
        if (participant.state() == state) {
            filtered << participant;
        }
    }
    return filtered;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Participants &participants)
{
    argument.beginArray();
    while (!argument.atEnd()) {
        QVariantMap props;

        // we are casting from a QVariantList, so the inner argument is a QVariant and not the map directly
        // that's why this intermediate QVariant cast is needed
        QVariant variant;
        argument >> variant;
        QDBusArgument innerArgument = variant.value<QDBusArgument>();
        if (!innerArgument.atEnd()) {
            innerArgument >> props;
        }
        participants << Participant::fromProperties(props);
    }
    argument.endArray();
    return argument;
}

}
