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

#include "voiceevent.h"
#include "voiceevent_p.h"

namespace History
{

// ------------- VoiceEventPrivate ------------------------------------------------

VoiceEventPrivate::VoiceEventPrivate(const QString &theAccountId,
                                   const QString &theThreadId,
                                   const QString &theEventId,
                                   const QString &theSender,
                                   const QDateTime &theTimestamp,
                                   bool theNewEvent,
                                   bool theMissed,
                                   const QTime &theDuration)
    : EventPrivate(theAccountId, theThreadId, theEventId, theSender, theTimestamp, theNewEvent),
      missed(theMissed), duration(theDuration)
{
}

VoiceEventPrivate::~VoiceEventPrivate()
{
}

// ------------- VoiceEvent -------------------------------------------------------

VoiceEvent::VoiceEvent(const QString &accountId,
                     const QString &threadId,
                     const QString &eventId,
                     const QString &sender,
                     const QDateTime &timestamp,
                     bool newEvent,
                     bool missed,
                     const QTime &duration)
    : Event(*new VoiceEventPrivate(accountId, threadId, eventId, sender, timestamp, newEvent, missed, duration))
{
}

VoiceEvent::~VoiceEvent()
{
}

EventType VoiceEvent::type() const
{
    return EventTypeVoice;
}

QVariantMap VoiceEvent::properties() const
{
    Q_D(const VoiceEvent);

    QVariantMap map = Event::properties();

    map[FieldMissed] = d->missed;
    map[FieldDuration] = d->duration;

    return map;
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

}
