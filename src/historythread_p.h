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
                         const HistoryItemPtr &theLastItem,
                         int theCount,
                         int theUnreadCount);
    virtual ~HistoryThreadPrivate();


    QString accountId;
    QString threadId;
    QStringList participants;
    HistoryItemPtr lastItem;
    int count;
    int unreadCount;

    HistoryThread *q_ptr;
};


#endif // HISTORYTHREAD_P_H
