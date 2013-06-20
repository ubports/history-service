#ifndef HISTORYTHREAD_P_H
#define HISTORYTHREAD_P_H

#include "historythread.h"

class HistoryThreadPrivate
{
    Q_DECLARE_PUBLIC(HistoryThread)
public:
    HistoryThreadPrivate();
    HistoryThreadPrivate(const QString &theAccountId,
                       const QString &theThreadId,
                       const QStringList &theParticipants,
                       const QString &theSender,
                       const QDateTime &theTimestamp);
    virtual ~HistoryThreadPrivate();


    QString accountId;
    QString threadId;
    QString itemId;
    QString sender;
    QString receiver;
    QDateTime timestamp;

    HistoryThread *q_ptr;
};


#endif // HISTORYTHREAD_P_H
