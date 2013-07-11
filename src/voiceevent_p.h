#ifndef HISTORY_VOICEEVENT_P_H
#define HISTORY_VOICEEVENT_P_H

#include "event_p.h"

namespace History
{

class VoiceEventPrivate : public EventPrivate
{
public:
    VoiceEventPrivate();
    VoiceEventPrivate(const QString &theAccountId,
                    const QString &theThreadId,
                    const QString &theItemId,
                    const QString &theSender,
                    const QDateTime &theTimestamp,
                    bool theNewItem,
                    bool theMissed,
                    const QTime &theDuration = QTime());
    ~VoiceEventPrivate();
    bool missed;
    QTime duration;
};

}

#endif // HISTORY_VOICEEVENT_P_H
