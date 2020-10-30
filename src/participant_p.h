/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef HISTORY_PARTICIPANT_P_H
#define HISTORY_PARTICIPANT_P_H

#include <QString>
#include <QVariantMap>

namespace History
{

class Participant;

class ParticipantPrivate
{
public:
    explicit ParticipantPrivate();
    ParticipantPrivate(const QString &theAccountId,
                       const QString &theIdentifier,
                       const QString &theContactId = QString(),
                       const QString &theAlias = QString(),
                       const QString &theAvatar = QString(),
                       uint theState = 0,
                       uint theRoles = 0,
                       const QVariantMap &theDetailProperties = QVariantMap());
    virtual ~ParticipantPrivate();

    QString accountId;
    QString identifier;
    QString contactId;
    QString alias;
    QString avatar;
    uint state;
    uint roles;
    QVariantMap detailProperties;
};

}

#endif // HISTORY_PARTICIPANT_P_H
