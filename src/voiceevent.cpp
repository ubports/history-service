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
                                   const QTime &theDuration)
    : EventPrivate(theAccountId, theThreadId, theEventId, theSender, theTimestamp, theNewEvent),
      missed(theMissed), duration(theDuration)
{
}

VoiceEventPrivate::~VoiceEventPrivate()
{
}

// ------------- VoiceEvent -------------------------------------------------------

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
