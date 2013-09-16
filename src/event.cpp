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

#include "event.h"
#include "event_p.h"

namespace History
{

// ------------- EventPrivate ------------------------------------------------

EventPrivate::EventPrivate(const QString &theAccountId,
                                       const QString &theThreadId,
                                       const QString &theEventId,
                                       const QString &theSenderId,
                                       const QDateTime &theTimestamp,
                                       bool theNewEvent) :
    accountId(theAccountId), threadId(theThreadId), eventId(theEventId),
    senderId(theSenderId), timestamp(theTimestamp), newEvent(theNewEvent)
{
}

EventPrivate::~EventPrivate()
{
}

// ------------- Event -------------------------------------------------------

/*!
 * \class Event
 *
 * \brief The Event class provides the base class for all events stored
 *  and loaded from the history backends.
 *
 *  This class should not be used directly and instead
 *  the derived classes should be used.
 *
 * \sa TextEvent, VoiceEvent
 */

/*!
 * \fn Event::EventType Event::type() const
 * \brief Returns the type of this event.
 */

/*!
  \internal
 * \brief Constructor to be used by derived classes to pass a EventPrivate instance
 * \param p The instance of the private class;
 */
Event::Event(EventPrivate &p)
    : d_ptr(&p)
{
}

Event::~Event()
{
}

/*!
 * \brief Returns the account ID this event belongs to.
 */
QString Event::accountId() const
{
    Q_D(const Event);
    return d->accountId;
}

/*!
 * \brief Returns the ID of the communication thread this event belongs to.
 * \sa HistoryThread
 */
QString Event::threadId() const
{
    Q_D(const Event);
    return d->threadId;
}

/*!
 * \brief Returns the ID that uniquely identifies this event.
 */
QString Event::eventId() const
{
    Q_D(const Event);
    return d->eventId;
}

/*!
 * \brief Returns the ID of the sender of this event.
 */
QString Event::senderId() const
{
    Q_D(const Event);
    return d->senderId;
}

/*!
 * \brief Returns the timestamp of when the event happened.
 */
QDateTime Event::timestamp() const
{
    Q_D(const Event);
    return d->timestamp;
}

/*!
 * \brief Returns whether the event is new (not yet seen by the user).
 * \return
 */
bool Event::newEvent() const
{
    Q_D(const Event);
    return d->newEvent;
}

QVariantMap Event::properties() const
{
    Q_D(const Event);

    QVariantMap map;

    map[FieldAccountId] = d->accountId;
    map[FieldThreadId] = d->threadId;
    map[FieldEventId] = d->eventId;
    map[FieldSenderId] = d->senderId;
    map[FieldTimestamp] = d->timestamp;
    map[FieldNewEvent] = d->newEvent;
    map[FieldType] = type();

    return map;
}

}
