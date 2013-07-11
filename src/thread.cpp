#include "thread.h"
#include "thread_p.h"

namespace History
{

// ------------- ThreadPrivate ------------------------------------------------

ThreadPrivate::ThreadPrivate()
{
}

ThreadPrivate::ThreadPrivate(const QString &theAccountId,
                                           const QString &theThreadId, EventType theType,
                                           const QStringList &theParticipants,
                                           const EventPtr &theLastEvent,
                                           int theCount,
                                           int theUnreadCount) :
    accountId(theAccountId), threadId(theThreadId), type(theType), participants(theParticipants),
    lastEvent(theLastEvent), count(theCount), unreadCount(theUnreadCount)
{
}

ThreadPrivate::~ThreadPrivate()
{
}

// ------------- Thread ------------------------------------------------------

Thread::Thread()
    : d_ptr(new ThreadPrivate())
{
}

Thread::Thread(const QString &accountId,
               const QString &threadId, EventType type,
               const QStringList &participants,
               const EventPtr &lastEvent,
               int count,
               int unreadCount)
: d_ptr(new ThreadPrivate(accountId, threadId, type, participants, lastEvent, count, unreadCount))
{
}

Thread::~Thread()
{
}

QString Thread::accountId() const
{
    Q_D(const Thread);
    return d->accountId;
}

QString Thread::threadId() const
{
    Q_D(const Thread);
    return d->threadId;
}

EventType Thread::type() const
{
    Q_D(const Thread);
    return d->type;
}

QStringList Thread::participants() const
{
    Q_D(const Thread);
    return d->participants;
}

EventPtr Thread::lastEvent() const
{
    Q_D(const Thread);
    return d->lastEvent;
}

int Thread::count() const
{
    Q_D(const Thread);
    return d->count;
}

int Thread::unreadCount() const
{
    Q_D(const Thread);
    return d->unreadCount;
}

}
