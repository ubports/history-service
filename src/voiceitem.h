#ifndef VOICEITEM_H
#define VOICEITEM_H

#include <HistoryItem>

class VoiceItemPrivate;

class VoiceItem : public HistoryItem
{
    Q_DECLARE_PRIVATE(VoiceItem)

public:
    VoiceItem();
    VoiceItem(const QString &accountId,
             const QString &threadId,
             const QString &itemId,
             const QString &sender,
             const QDateTime &timestamp,
             bool missed,
             const QTime &duration = QTime());
    ~VoiceItem();

    HistoryItem::ItemType type() const;

    bool missed() const;
    QTime duration() const;
};

#endif

