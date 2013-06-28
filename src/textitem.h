#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <HistoryItem>

class TextItemPrivate;

class TextItem : public HistoryItem
{
    Q_DECLARE_PRIVATE(TextItem)

public:
    enum MessageFlag
    {
        Pending,
        Delivered
    };

    Q_DECLARE_FLAGS(MessageFlags, MessageFlag)

    enum MessageType
    {
        TextMessage,
        MultiPartMessage
    };

    TextItem();
    TextItem(const QString &accountId,
             const QString &threadId,
             const QString &itemId,
             const QString &sender,
             const QDateTime &timestamp,
             bool newItem,
             const QString &message,
             MessageType messageType,
             MessageFlags messageFlags,
             const QDateTime &readTimestamp);
    ~TextItem();

    HistoryItem::ItemType type() const;

    QString message() const;
    MessageType messageType() const;
    MessageFlags messageFlags() const;
    QDateTime readTimestamp() const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TextItem::MessageFlags)

#endif
