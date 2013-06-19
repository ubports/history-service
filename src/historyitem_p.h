#ifndef HISTORYITEM_P_H
#define HISTORYITEM_P_H

#include "historyitem.h"
#include <QDateTime>
#include <QString>

class HistoryItemPrivate
{
    Q_DECLARE_PUBLIC(HistoryItem)
public:
    HistoryItemPrivate();
    HistoryItemPrivate(const QString &theAccountId,
                       const QString &theThreadId,
                       const QString &theItemId,
                       const QString &theSender,
                       const QDateTime &theTimestamp);
    virtual ~HistoryItemPrivate();


    QString accountId;
    QString threadId;
    QString itemId;
    QString sender;
    QString receiver;
    QDateTime timestamp;

    HistoryItem *q_ptr;
};

#endif // HISTORYITEM_P_H
