#ifndef HISTORY_THREAD_P_H
#define HISTORY_THREAD_P_H

#include <QString>
#include "types.h"

namespace History
{

class Thread;

class ThreadPrivate
{
public:
    ThreadPrivate();
    ThreadPrivate(const QString &theAccountId,
                         const QString &theThreadId,
                         EventType theType,
                         const QStringList &theParticipants,
                         const EventPtr &theLastEvent,
                         int theCount,
                         int theUnreadCount);
    virtual ~ThreadPrivate();

    QString accountId;
    QString threadId;
    QStringList participants;
    EventType type;
    EventPtr lastEvent;
    int count;
    int unreadCount;
};

}

#endif // HISTORY_THREAD_P_H
