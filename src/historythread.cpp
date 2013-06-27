#include "historythread.h"
#include "historythread_p.h"

// ------------- HistoryThreadPrivate ------------------------------------------------

HistoryThreadPrivate::HistoryThreadPrivate()
{
}

HistoryThreadPrivate::HistoryThreadPrivate(const QString &theAccountId,
                                           const QString &theThreadId,
                                           const QStringList &theParticipants,
                                           const HistoryItemPtr &theLastItem,
                                           int theCount,
                                           int theUnreadCount) :
    accountId(theAccountId), threadId(theThreadId), participants(theParticipants),
    lastItem(theLastItem), count(theCount), unreadCount(theUnreadCount)
{
}

HistoryThreadPrivate::~HistoryThreadPrivate()
{
}

// ------------- HistoryThread ------------------------------------------------------

HistoryThread::HistoryThread()
    : d_ptr(new HistoryThreadPrivate())
{
    d_ptr->q_ptr = this;
}

HistoryThread::HistoryThread(const QString &accountId,
                             const QString &threadId,
                             const QStringList &participants,
                             const HistoryItemPtr &lastItem,
                             int count,
                             int unreadCount)
: d_ptr(new HistoryThreadPrivate(accountId, threadId, participants, lastItem, count, unreadCount))
{
    d_ptr->q_ptr = this;
}

HistoryThread::~HistoryThread()
{
}

QString HistoryThread::accountId() const
{
    Q_D(const HistoryThread);
    return d->accountId;
}

QString HistoryThread::threadId() const
{
    Q_D(const HistoryThread);
    return d->threadId;
}

QStringList HistoryThread::participants() const
{
    Q_D(const HistoryThread);
    return d->participants;
}

HistoryItemPtr HistoryThread::lastItem() const
{
    Q_D(const HistoryThread);
    return d->lastItem;
}

int HistoryThread::count() const
{
    Q_D(const HistoryThread);
    return d->count;
}

int HistoryThread::unreadCount() const
{
    Q_D(const HistoryThread);
    return d->unreadCount;
}
