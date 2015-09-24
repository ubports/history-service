#include "participant.h"
#include "participant_p.h"
#include "types.h"
#include <QDebug>

namespace History
{

ParticipantPrivate::ParticipantPrivate()
{

}

ParticipantPrivate::ParticipantPrivate(const QString &theAccountId,
                                       const QString &theIdentifier,
                                       const QString &theContactId,
                                       const QString &theAlias,
                                       const QString &theAvatar) :
    accountId(theAccountId), identifier(theIdentifier), contactId(theContactId), alias(theAlias), avatar(theAvatar)
{
}

ParticipantPrivate::~ParticipantPrivate()
{
}

Participant::Participant()
    : d_ptr(new ParticipantPrivate())
{
}

Participant::Participant(const QString &accountId, const QString &identifier, const QString &contactId, const QString &alias, const QString &avatar)
    : d_ptr(new ParticipantPrivate(accountId, identifier, contactId, alias, avatar))
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

bool Participant::isNull() const
{
    Q_D(const Participant);
    return d->accountId.isNull() && d->identifier.isNull();
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

    return Participant(accountId, identifier, contactId, alias, avatar);
}

QStringList Participants::identifiers() const
{
    QStringList result;
    Q_FOREACH(const Participant &participant, *this) {
        result << participant.identifier();
    }
    return result;
}

Participants Participants::fromVariantList(const QVariantList &list)
{
    Participants participants;
    Q_FOREACH(const QVariant& entry, list) {
        participants << Participant::fromProperties(entry.toMap());
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

}
