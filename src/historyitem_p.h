#ifndef HISTORYITEM_P_H
#define HISTORYITEM_P_H

#include <QDateTime>
#include <QString>

class HistoryItem;

class HistoryItemPrivate
{
    Q_DECLARE_PUBLIC(HistoryItem)
public:
    HistoryItemPrivate();
    HistoryItemPrivate(const QString &theAccountId,
                       const QString &theThreadId,
                       const QString &theItemId,
                       const QString &theSender,
                       const QDateTime &theTimestamp,
                       bool theNewItem);
    virtual ~HistoryItemPrivate();


    QString accountId;
    QString threadId;
    QString itemId;
    QString sender;
    QString receiver;
    QDateTime timestamp;
    bool newItem;

    HistoryItem *q_ptr;
};

#endif // HISTORYITEM_P_H
