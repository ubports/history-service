#ifndef TEXTITEM_P_H
#define TEXTITEM_P_H

#include "historyitem_p.h"

class TextItem;

class TextItemPrivate : public HistoryItemPrivate
{
    Q_DECLARE_PUBLIC(TextItem)

public:
    TextItemPrivate();
    TextItemPrivate(const QString &theAccountId,
                    const QString &theThreadId,
                    const QString &theItemId,
                    const QString &theSender,
                    const QDateTime &theTimestamp,
                    bool theNewItem,
                    const QString &theMessage,
                    TextItem::MessageType theMessageType,
                    TextItem::MessageFlags theMessageFlags,
                    const QDateTime &theReadTimestamp);
    ~TextItemPrivate();
    QString message;
    TextItem::MessageType messageType;
    TextItem::MessageFlags messageFlags;
    QDateTime readTimestamp;
};

#endif // TEXTITEM_P_H
