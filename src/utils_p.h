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

#ifndef UTILS_P_H
#define UTILS_P_H

#include "types.h"
#include "thread.h"

namespace History {

class Utils
{
public:
    static MatchFlags matchFlagsForAccount(const QString &accountId);
    static QString protocolFromAccountId(const QString &accountId);
    static bool compareIds(const QString &accountId, const QString &id1, const QString & id2);
    static bool compareParticipants(const QStringList &participants1, const QStringList &participants2, MatchFlags flags);
    static bool compareNormalizedParticipants(const QStringList &participants1, const QStringList &participants2, MatchFlags flags);
    static bool shouldGroupThread(const Thread &thread);
    static bool shouldIncludeParticipants(const QString &accountId);
    static QString normalizeId(const QString &accountId, const QString &id);
    static QVariant getUserValue(const QString &interface, const QString &propName);

private:
    Utils();
};

}

#endif // UTILS_P_H
