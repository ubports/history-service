#ifndef TEXTITEM_P_H
#define TEXTITEM_P_H

#include "historyitem_p.h"
#include "textitem.h"

class TextItemPrivate : public HistoryItemPrivate
{
public:
    TextItemPrivate();
    TextItemPrivate(const QString &theAccountId,
                    const QString &theThreadId,
                    const QString &theItemId,
                    const QString &theSender,
                    const QDateTime &theTimestamp,
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
