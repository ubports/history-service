#ifndef HISTORY_TEXTEVENT_P_H
#define HISTORY_TEXTEVENT_P_H

#include "event_p.h"
#include "types.h"

namespace History
{

class TextEvent;

class TextEventPrivate : public EventPrivate
{
public:
    TextEventPrivate();
    TextEventPrivate(const QString &theAccountId,
                    const QString &theThreadId,
                    const QString &theItemId,
                    const QString &theSender,
                    const QDateTime &theTimestamp,
                    bool theNewItem,
                    const QString &theMessage,
                    MessageType theMessageType,
                    MessageFlags theMessageFlags,
                    const QDateTime &theReadTimestamp);
    ~TextEventPrivate();
    QString message;
    MessageType messageType;
    MessageFlags messageFlags;
    QDateTime readTimestamp;
};

}

#endif // HISTORY_TEXTEVENT_P_H
