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

EventPrivate::EventPrivate()
{
}

EventPrivate::EventPrivate(const QString &theAccountId,
                                       const QString &theThreadId,
                                       const QString &theEventId,
                                       const QString &theSenderId,
                                       const QDateTime &theTimestamp,
                                       bool theNewEvent,
                                       const Participants &theParticipants) :
    accountId(theAccountId), threadId(theThreadId), eventId(theEventId),
    senderId(theSenderId), timestamp(theTimestamp), newEvent(theNewEvent),
    participants(theParticipants)
{
}

EventPrivate::~EventPrivate()
{
}

QVariantMap EventPrivate::properties() const
{
    QVariantMap map;

    map[FieldAccountId] = accountId;
    map[FieldThreadId] = threadId;
    map[FieldEventId] = eventId;
    map[FieldSenderId] = senderId;
    map[FieldTimestamp] = timestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz");
    map[FieldDate] = timestamp.date().toString(Qt::ISODate);
    map[FieldNewEvent] = newEvent;
    map[FieldType] = type();
    map[FieldParticipants] = participants.toVariantList();

    return map;
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
 * \brief Constructs an empty Event
 */
Event::Event()
    : d_ptr(new EventPrivate())
{

}

/*!
 * \brief Constructs an Event by copying the data from another one.
 * \param other The item to be copied;
 */
Event::Event(const Event &other)
    : d_ptr(other.d_ptr->clone())
{
}

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
 * \brief Assign operator for the Event class
 * \param other The event to be copied;
 */
Event& Event::operator=(const Event &other)
{
    if (&other == this) {
        return *this;
    }

    d_ptr = QSharedPointer<EventPrivate>(other.d_ptr->clone());
    return *this;
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
 * \sa Event::setNewEvent()
 */
bool Event::newEvent() const
{
    Q_D(const Event);
    return d->newEvent;
}

/*!
 * \brief Set whether this event is new (not yet seen by the user).
 * \param value True if the event is new. False otherwise.
 * \sa Event::newEvent()
 */
void Event::setNewEvent(bool value)
{
    Q_D(Event);
    d->newEvent = value;
}

/*!
 * \brief Returns the type of this event.
 */
EventType Event::type() const
{
    Q_D(const Event);
    return d->type();
}

Participants Event::participants() const
{
    Q_D(const Event);
    return d->participants;
}

/*!
 * \brief Returns the event properties
 */
QVariantMap Event::properties() const
{
    Q_D(const Event);
    return d->properties();
}

/*!
 * \brief Return whether this event is a null event.
 */
bool Event::isNull() const
{
    Q_D(const Event);
    return d->accountId.isNull() && d->threadId.isNull() && d->eventId.isNull();
}

/*!
 * \brief Compare this event with another one.
 * \param other The other event;
 */
bool Event::operator==(const Event &other) const
{
    Q_D(const Event);
    if (type() != other.type()) {
        return false;
    }
    if (d->accountId != other.d_ptr->accountId) {
        return false;
    }
    if (d->threadId != other.d_ptr->threadId) {
        return false;
    }
    if (d->eventId != other.d_ptr->eventId) {
        return false;
    }
    return true;
}

bool Event::operator!=(const Event &other) const
{
    return !(*this == other);
}

bool Event::operator<(const Event &other) const
{
    QString selfData = accountId() + threadId() + eventId();
    QString otherData = other.accountId() + other.threadId() + other.eventId();
    return selfData < otherData;
}

}
