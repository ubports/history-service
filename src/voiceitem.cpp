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
                                   bool theMissed,
                                   const QTime &theDuration)
    : HistoryItemPrivate(theAccountId, theThreadId, theItemId, theSender, theTimestamp),
      missed(theMissed), duration(theDuration)
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
                     bool missed,
                     const QTime &duration)
    : HistoryItem(*new VoiceItemPrivate(accountId, threadId, itemId, sender, timestamp, missed, duration))

{
}

VoiceItem::~VoiceItem()
{
}

HistoryItem::ItemType VoiceItem::type() const
{
    return HistoryItem::VoiceItem;
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

