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

#include "textevent.h"
#include "textevent_p.h"

namespace History {

// ------------- TextEventPrivate ------------------------------------------------

TextEventPrivate::TextEventPrivate()
{
}

TextEventPrivate::TextEventPrivate(const QString &theAccountId,
                                 const QString &theThreadId,
                                 const QString &theEventId,
                                 const QString &theSender,
                                 const QDateTime &theTimestamp,
                                 bool theNewEvent,
                                 const QString &theMessage,
                                 MessageType theMessageType,
                                 MessageFlags theMessageFlags,
                                 const QDateTime &theReadTimestamp) :
    EventPrivate(theAccountId, theThreadId, theEventId, theSender, theTimestamp, theNewEvent),
    message(theMessage), messageType(theMessageType), messageFlags(theMessageFlags),
    readTimestamp(theReadTimestamp)
{
}

TextEventPrivate::~TextEventPrivate()
{
}

// ------------- TextEvent -------------------------------------------------------

TextEvent::TextEvent()
    : Event(*new TextEventPrivate())
{
}

TextEvent::TextEvent(const QString &accountId,
                   const QString &threadId,
                   const QString &eventId,
                   const QString &sender,
                   const QDateTime &timestamp,
                   bool newEvent,
                   const QString &message,
                   MessageType messageType,
                   MessageFlags messageFlags,
                   const QDateTime &readTimestamp)
    : Event(*new TextEventPrivate(accountId, threadId, eventId, sender, timestamp, newEvent,
                                       message, messageType, messageFlags, readTimestamp))
{
}

TextEvent::~TextEvent()
{
}

EventType TextEvent::type() const
{
    return EventTypeText;
}

QVariantMap TextEvent::properties() const
{
    Q_D(const TextEvent);

    QVariantMap map = Event::properties();

    map["message"] = d->message;
    map["messageType"] = (int)d->messageType;
    map["messageFlags"] = (int)d->messageFlags;
    map["readTimestamp"] = d->readTimestamp;

    return map;
}

QString TextEvent::message() const
{
    Q_D(const TextEvent);
    return d->message;
}

MessageType TextEvent::messageType() const
{
    Q_D(const TextEvent);
    return d->messageType;
}

MessageFlags TextEvent::messageFlags() const
{
    Q_D(const TextEvent);
    return d->messageFlags;
}

QDateTime TextEvent::readTimestamp() const
{
    Q_D(const TextEvent);
    return d->readTimestamp;
}

}
