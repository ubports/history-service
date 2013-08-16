/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#ifndef ITEMFACTORY_H
#define ITEMFACTORY_H

#include <QMap>
#include <QObject>
#include <QTime>
#include <QScopedPointer>
#include "types.h"

namespace History
{

class ItemFactoryPrivate;

class ItemFactory
{
    Q_DECLARE_PRIVATE(ItemFactory)

public:
    static ItemFactory *instance();
    ThreadPtr createThread(const QString &accountId,
                           const QString &threadId,
                           EventType type,
                           const QStringList &participants,
                           const EventPtr &lastEvent = EventPtr(),
                           int count = 0,
                           int unreadCount = 0);
    TextEventPtr createTextEvent(const QString &accountId,
                                 const QString &threadId,
                                 const QString &eventId,
                                 const QString &senderId,
                                 const QDateTime &timestamp,
                                 bool newEvent,
                                 const QString &message,
                                 MessageType messageType,
                                 MessageFlags messageFlags,
                                 const QDateTime &readTimestamp,
                                 const QString &subject = QString(),
                                 const TextEventAttachments &attachments = TextEventAttachments());
    VoiceEventPtr createVoiceEvent(const QString &accountId,
                                   const QString &threadId,
                                   const QString &eventId,
                                   const QString &senderId,
                                   const QDateTime &timestamp,
                                   bool newEvent,
                                   bool missed,
                                   const QTime &duration = QTime());

    ThreadPtr cachedThread(const QString &accountId,
                           const QString &threadId,
                           EventType type);
    EventPtr cachedEvent(const QString &accountId,
                         const QString &threadId,
                         const QString &eventId,
                         EventType type);

private:
    explicit ItemFactory();
    QScopedPointer<ItemFactoryPrivate> d_ptr;
};

}

#endif // ITEMFACTORY_H
