/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#include "voiceevent.h"
#include "voiceevent_p.h"

namespace History
{

// ------------- VoiceEventPrivate ------------------------------------------------

VoiceEventPrivate::VoiceEventPrivate()
{
}

VoiceEventPrivate::VoiceEventPrivate(const QString &theAccountId,
                                   const QString &theThreadId,
                                   const QString &theEventId,
                                   const QString &theSender,
                                   const QDateTime &theTimestamp,
                                   bool theNewEvent,
                                   bool theMissed,
                                   const QTime &theDuration,
                                   const QString &theRemoteParticipant,
                                   const Participants &theParticipants)
    : EventPrivate(theAccountId, theThreadId, theEventId, theSender, theTimestamp, theNewEvent, theParticipants),
      missed(theMissed), duration(theDuration), remoteParticipant(theRemoteParticipant)
{
}

VoiceEventPrivate::~VoiceEventPrivate()
{
}

EventType VoiceEventPrivate::type() const
{
    return EventTypeVoice;
}

QVariantMap VoiceEventPrivate::properties() const
{
    QVariantMap map = EventPrivate::properties();

    map[FieldMissed] = missed;
    map[FieldDuration] = QTime(0,0,0,0).secsTo(duration);
    map[FieldRemoteParticipant] = remoteParticipant;

    return map;
}



// ------------- VoiceEvent -------------------------------------------------------

HISTORY_EVENT_DEFINE_COPY(VoiceEvent, EventTypeVoice)

VoiceEvent::VoiceEvent()
    : Event(*new VoiceEventPrivate())
{
}

VoiceEvent::VoiceEvent(const QString &accountId,
                     const QString &threadId,
                     const QString &eventId,
                     const QString &sender,
                     const QDateTime &timestamp,
                     bool newEvent,
                     bool missed,
                     const QTime &duration,
                     const QString &remoteParticipant,
                     const Participants &participants)
    : Event(*new VoiceEventPrivate(accountId, threadId, eventId, sender, timestamp, newEvent, missed, duration, remoteParticipant, participants))
{
}

VoiceEvent::~VoiceEvent()
{
}

bool VoiceEvent::missed() const
{
    Q_D(const VoiceEvent);
    return d->missed;
}

QTime VoiceEvent::duration() const
{
    Q_D(const VoiceEvent);
    return d->duration;
}

QString VoiceEvent::remoteParticipant() const
{
    Q_D(const VoiceEvent);
    return d->remoteParticipant;
}

Event VoiceEvent::fromProperties(const QVariantMap &properties)
{
    Event event;
    if (properties.isEmpty()) {
        return event;
    }
    QString accountId = properties[FieldAccountId].toString();
    QString threadId = properties[FieldThreadId].toString();
    QString eventId = properties[FieldEventId].toString();
    QString senderId = properties[FieldSenderId].toString();
    QDateTime timestamp = QDateTime::fromString(properties[FieldTimestamp].toString(), Qt::ISODate);
    bool newEvent = properties[FieldNewEvent].toBool();
    Participants participants = Participants::fromVariant(properties[FieldParticipants]);
    bool missed = properties[FieldMissed].toBool();
    QTime duration = QTime(0,0,0).addSecs(properties[FieldDuration].toInt());
    QString remoteParticipant = properties[FieldRemoteParticipant].toString();
    event = VoiceEvent(accountId, threadId, eventId, senderId, timestamp, newEvent,
                             missed, duration, remoteParticipant, participants);
    return event;
}

}
