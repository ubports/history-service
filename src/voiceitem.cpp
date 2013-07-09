#include "voiceitem.h"
#include "voiceitem_p.h"

// ------------- VoiceItemPrivate ------------------------------------------------

VoiceItemPrivate::VoiceItemPrivate()
{
}

VoiceItemPrivate::VoiceItemPrivate(const QString &theAccountId,
                                   const QString &theThreadId,
                                   const QString &theItemId,
                                   const QString &theSender,
                                   const QDateTime &theTimestamp,
                                   bool theNewItem,
                                   bool theMissed,
                                   const QTime &theDuration)
    : HistoryItemPrivate(theAccountId, theThreadId, theItemId, theSender, theTimestamp, theNewItem),
      missed(theMissed), duration(theDuration)
{
}

VoiceItemPrivate::~VoiceItemPrivate()
{
}

// ------------- VoiceItem -------------------------------------------------------

VoiceItem::VoiceItem()
    : HistoryItem(*new VoiceItemPrivate())
{
}

VoiceItem::VoiceItem(const QString &accountId,
                     const QString &threadId,
                     const QString &itemId,
                     const QString &sender,
                     const QDateTime &timestamp,
                     bool newItem,
                     bool missed,
                     const QTime &duration)
    : HistoryItem(*new VoiceItemPrivate(accountId, threadId, itemId, sender, timestamp, newItem, missed, duration))

{
}

VoiceItem::~VoiceItem()
{
}

HistoryItem::ItemType VoiceItem::type() const
{
    return HistoryItem::ItemTypeVoice;
}

bool VoiceItem::missed() const
{
    Q_D(const VoiceItem);
    return d->missed;
}

QTime VoiceItem::duration() const
{
    Q_D(const VoiceItem);
    return d->duration;
}

