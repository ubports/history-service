#include "textevent.h"
#include "textevent_p.h"

namespace History {

// ------------- TextEventPrivate ------------------------------------------------

TextEventPrivate::TextEventPrivate()
{
}

TextEventPrivate::TextEventPrivate(const QString &theAccountId,
                                 const QString &theThreadId,
                                 const QString &theItemId,
                                 const QString &theSender,
                                 const QDateTime &theTimestamp,
                                 bool theNewItem,
                                 const QString &theMessage,
                                 MessageType theMessageType,
                                 MessageFlags theMessageFlags,
                                 const QDateTime &theReadTimestamp) :
    EventPrivate(theAccountId, theThreadId, theItemId, theSender, theTimestamp, theNewItem),
    message(theMessage), messageType(theMessageType), messageFlags(theMessageFlags),
    readTimestamp(theReadTimestamp)
{
}

TextEventPrivate::~TextEventPrivate()
{
}

// ------------- TextEvent -------------------------------------------------------

TextEvent::TextEvent()
    : Event(*new TextEventPrivate())
{
}

TextEvent::TextEvent(const QString &accountId,
                   const QString &threadId,
                   const QString &eventId,
                   const QString &sender,
                   const QDateTime &timestamp,
                   bool newEvent,
                   const QString &message,
                   MessageType messageType,
                   MessageFlags messageFlags,
                   const QDateTime &readTimestamp)
    : Event(*new TextEventPrivate(accountId, threadId, eventId, sender, timestamp, newEvent,
                                       message, messageType, messageFlags, readTimestamp))
{
}

TextEvent::~TextEvent()
{
}

EventType TextEvent::type() const
{
    return EventTypeText;
}

QString TextEvent::message() const
{
    Q_D(const TextEvent);
    return d->message;
}

MessageType TextEvent::messageType() const
{
    Q_D(const TextEvent);
    return d->messageType;
}

MessageFlags TextEvent::messageFlags() const
{
    Q_D(const TextEvent);
    return d->messageFlags;
}

QDateTime TextEvent::readTimestamp() const
{
    Q_D(const TextEvent);
    return d->readTimestamp;
}

}
