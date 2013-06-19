#ifndef VOICEITEM_P_H
#define VOICEITEM_P_H

#include "historyitem_p.h"
#include "voiceitem.h"

class VoiceItemPrivate : public HistoryItemPrivate
{
    Q_DECLARE_PUBLIC(VoiceItem)

public:
    VoiceItemPrivate();
    VoiceItemPrivate(const QString &theAccountId,
                    const QString &theThreadId,
                    const QString &theItemId,
                    const QString &theSender,
                    const QDateTime &theTimestamp,
                    bool theMissed,
                    const QTime &theDuration = QTime());
    ~VoiceItemPrivate();
    bool missed;
    QTime duration;
};
#endif // VOICEITEM_P_H
