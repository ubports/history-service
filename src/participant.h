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

#ifndef HISTORY_PARTICIPANT_H
#define HISTORY_PARTICIPANT_H

#include <QDBusArgument>
#include <QList>
#include <QSharedPointer>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

namespace History
{

class ParticipantPrivate;

class Participant
{
    Q_DECLARE_PRIVATE(Participant)
public:
    explicit Participant();
    Participant(const QString &accountId,
                const QString &identifier,
                const QString &contactId = QString::null,
                const QString &alias = QString::null,
                const QString &avatar = QString::null,
                uint state = 0,
                const QVariantMap &detailProperties = QVariantMap());
    Participant(const Participant &other);
    Participant& operator=(const Participant &other);
    virtual ~Participant();

    QString accountId() const;
    QString identifier() const;
    QString contactId() const;
    QString alias() const;
    QString avatar() const;
    uint state() const;
    QVariantMap detailProperties() const;

    bool isNull() const;
    bool operator==(const Participant &other) const;
    bool operator<(const Participant &other) const;

    virtual QVariantMap properties() const;

    static Participant fromProperties(const QVariantMap &properties);

protected:
    QSharedPointer<ParticipantPrivate> d_ptr;
};

// define the participants list with toVariantList() and fromVariantList() helpers
class Participants : public QList<Participant>
{
public:
    QStringList identifiers() const;
    static Participants fromVariant(const QVariant &variant);
    static Participants fromVariantList(const QVariantList &list);
    static Participants fromStringList(const QStringList &list);
    QVariantList toVariantList() const;

};

const QDBusArgument &operator>>(const QDBusArgument &argument, Participants &participants);

}

#endif // HISTORY_PARTICIPANT_H
