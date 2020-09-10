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

#include <unistd.h>
#include "utils_p.h"
#include "phoneutils_p.h"
#include <QDebug>
#include <QStringList>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>
#include <QMap>

namespace History {

Utils::Utils()
{
}

// FIXME: find a better way to determine when accounts should be grouped
bool Utils::shouldGroupThread(const Thread &thread)
{
    if (protocolFromAccountId(thread.accountId()) == "multimedia") {
       return thread.chatType() != History::ChatTypeRoom;
    }
    return (matchFlagsForAccount(thread.accountId()) & MatchPhoneNumber);
}

MatchFlags Utils::matchFlagsForAccount(const QString &accountId)
{
    static QMap<QString, History::MatchFlags> protocolFlags;
    if (protocolFlags.isEmpty()) {
        protocolFlags["ofono"] = MatchPhoneNumber;
        protocolFlags["multimedia"] = MatchPhoneNumber;
        protocolFlags["sip"] = MatchPhoneNumber;
    }

    QString protocol = protocolFromAccountId(accountId);
    if (protocolFlags.contains(protocol)) {
        return protocolFlags[protocol];
    }

    // default to phone number matching for now
    return History::MatchCaseSensitive;
}

QString Utils::protocolFromAccountId(const QString &accountId)
{
    QStringList parsedId = accountId.split("/");
    if (parsedId.count() < 3) {
        return QString();
    }
    return parsedId[1];
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

bool Utils::compareParticipants(const QStringList &participants1, const QStringList &participants2, MatchFlags flags)
{
    // if list size is different, just return
    if (participants1.count() != participants2.count()) {
        return false;
    }

    if (flags & MatchPhoneNumber) {
        QStringList normalizedParticipants1;
        QStringList normalizedParticipants2;
        Q_FOREACH(const QString &participant, participants1) {
            normalizedParticipants1 << PhoneUtils::normalizePhoneNumber(participant);
        }
        Q_FOREACH(const QString &participant, participants2) {
            normalizedParticipants2 << PhoneUtils::normalizePhoneNumber(participant);
        }
        return compareNormalizedParticipants(normalizedParticipants1, normalizedParticipants2, flags);

    }

    return compareNormalizedParticipants(participants1, participants2, flags);
}

bool Utils::compareNormalizedParticipants(const QStringList &participants1, const QStringList &participants2, MatchFlags flags)
{
    QStringList mutableParticipants2 = participants2;
    // if list size is different, just return
    if (participants1.count() != participants2.count()) {
        return false;
    }

    // and now compare the lists
    bool found = true;
    Q_FOREACH(const QString &participant, participants1) {
        if (flags & MatchPhoneNumber) {
            // we need to iterate the list and call the phone number comparing function for
            // each participant from the given thread
            bool inList = false;
            QStringList::iterator it = mutableParticipants2.begin();
            while (it != mutableParticipants2.end()) {
                if (PhoneUtils::compareNormalizedPhoneNumbers(*it, participant)) {
                    inList = true;
                    mutableParticipants2.erase(it);
                    break;
                }
                ++it;
            }
            if (!inList) {
                found = false;
                break;
            }
        } else if (!mutableParticipants2.contains(participant)) {
            found = false;
            break;
        }
    }
    return found;
}

QString Utils::normalizeId(const QString &accountId, const QString &id)
{
    QString normalizedId = id;
    // for now we only normalize phone number IDs
    if (matchFlagsForAccount(accountId) & History::MatchPhoneNumber) {
        normalizedId = PhoneUtils::normalizePhoneNumber(id);
    }
    if (normalizedId.isEmpty()) {
        qWarning() << "Normalized phone number is empty:" << accountId << id;
        normalizedId = id;
    }
    return normalizedId;
}

QVariant Utils::getUserValue(const QString &interface, const QString &propName)
{
    QString uid = QString::number(getuid());
    QString activeUser = "/org/freedesktop/Accounts/User" + uid;

    QDBusInterface iface("org.freedesktop.Accounts",
                         activeUser,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::systemBus());
    QDBusReply<QVariant> reply = iface.call("Get", interface, propName);
    if (reply.isValid()) {
        return reply.value();
    } else {
        qWarning() << "Failed to get user property " << propName << " from AccountsService:" << reply.error().message();
    }
    return QVariant();
}

bool Utils::shouldIncludeParticipants(const Thread &thread)
{
    return shouldIncludeParticipants(thread.accountId(), thread.chatType());
}

bool Utils::shouldIncludeParticipants(const QString &accountId, const ChatType &type)
{
    // FIXME: this is obviously incorrect. we have to query the protocol files as a final solution
    if (protocolFromAccountId(accountId) == "irc") {
        return type != ChatTypeRoom;
    }
    return true;
}

}
