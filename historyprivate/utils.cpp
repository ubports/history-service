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

#include "utils_p.h"
#include "phoneutils_p.h"
#include <QStringList>
#include <QMap>

namespace History {

Utils::Utils()
{
}


MatchFlags Utils::matchFlagsForAccount(const QString &accountId)
{
    static QMap<QString, History::MatchFlags> protocolFlags;
    if (protocolFlags.isEmpty()) {
        protocolFlags["ofono"] = MatchPhoneNumber;
    }

    QString protocol = protocolFromAccountId(accountId);
    if (protocolFlags.contains(protocol)) {
        return protocolFlags[protocol];
    }

    // default to this value
    return History::MatchCaseSensitive;
}

QString Utils::protocolFromAccountId(const QString &accountId)
{
    QStringList parsedId = accountId.split("/");
    if (parsedId.count() < 3) {
        return QString::null;
    }
    return parsedId[2];
}

bool Utils::compareIds(const QString &accountId, const QString &id1, const QString &id2)
{
    MatchFlags matchFlags = matchFlagsForAccount(accountId);
    if (matchFlags & MatchPhoneNumber) {
        return PhoneUtils::comparePhoneNumbers(id1, id2);
    }

    if (matchFlags & MatchCaseInsensitive) {
        return id1.toLower() == id2.toLower();
    }

    return id1 == id2;
}

}
