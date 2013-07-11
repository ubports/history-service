#ifndef HISTORY_VOICEEVENT_H
#define HISTORY_VOICEEVENT_H

#include "event.h"

namespace History
{

class VoiceEventPrivate;

class VoiceEvent : public Event
{
    Q_DECLARE_PRIVATE(VoiceEvent)

public:
    VoiceEvent();
    VoiceEvent(const QString &accountId,
             const QString &threadId,
             const QString &eventId,
             const QString &sender,
             const QDateTime &timestamp,
             bool newEvent,
             bool missed,
             const QTime &duration = QTime());
    ~VoiceEvent();

    EventType type() const;

    bool missed() const;
    QTime duration() const;
};

}

#endif

