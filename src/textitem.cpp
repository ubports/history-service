#include "textitem.h"
#include "textitem_p.h"

// ------------- TextItemPrivate ------------------------------------------------

TextItemPrivate::TextItemPrivate()
{
}

TextItemPrivate::TextItemPrivate(const QString &theAccountId,
                                 const QString &theThreadId,
                                 const QString &theItemId,
                                 const QString &theSender,
                                 const QDateTime &theTimestamp,
                                 const QString &theMessage,
                                 TextItem::MessageType theMessageType,
                                 TextItem::MessageFlags theMessageFlags,
                                 const QDateTime &theReadTimestamp) :
    HistoryItemPrivate(theAccountId, theThreadId, theItemId, theSender, theTimestamp),
    message(theMessage), messageType(theMessageType), messageFlags(theMessageFlags),
    readTimestamp(theReadTimestamp)
{
}

TextItemPrivate::~TextItemPrivate()
{
}

// ------------- TextItem -------------------------------------------------------

TextItem::TextItem()
{
}

TextItem::TextItem(const QString &accountId,
                   const QString &threadId,
                   const QString &itemId,
                   const QString &sender,
                   const QDateTime &timestamp,
                   const QString &message,
                   MessageType messageType,
                   MessageFlags messageFlags,
                   const QDateTime &readTimestamp)
    : HistoryItem(*new TextItemPrivate(accountId, threadId, itemId, sender, timestamp,
                                       message, messageType, messageFlags, readTimestamp))
{
}

TextItem::~TextItem()
{
}

HistoryItem::ItemType TextItem::type() const
{
    return HistoryItem::TextItem;
}

QString TextItem::message() const
{
    Q_D(const TextItem);
    return d->message;
}

TextItem::MessageType TextItem::messageType() const
{
    Q_D(const TextItem);
    return d->messageType;
}

TextItem::MessageFlags TextItem::messageFlags() const
{
    Q_D(const TextItem);
    return d->messageFlags;
}

QDateTime TextItem::readTimestamp() const
{
    Q_D(const TextItem);
    return d->readTimestamp;
}
