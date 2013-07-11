#ifndef HISTORY_TEXTEVENT_H
#define HISTORY_TEXTEVENT_H

#include "event.h"

namespace History
{

class TextEventPrivate;

class TextEvent : public Event
{
    Q_DECLARE_PRIVATE(TextEvent)

public:
    TextEvent();
    TextEvent(const QString &accountId,
             const QString &threadId,
             const QString &eventId,
             const QString &sender,
             const QDateTime &timestamp,
             bool newEvent,
             const QString &message,
             MessageType messageType,
             MessageFlags messageFlags,
             const QDateTime &readTimestamp);
    ~TextEvent();

    EventType type() const;

    QString message() const;
    MessageType messageType() const;
    MessageFlags messageFlags() const;
    QDateTime readTimestamp() const;
};

}

#endif // HISTORY_TEXTEVENT_H
