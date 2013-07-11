#ifndef HISTORY_THREAD_H
#define HISTORY_THREAD_H

#include <QDateTime>
#include <QScopedPointer>
#include <QStringList>
#include "types.h"

namespace History
{

class ThreadPrivate;

class Thread
{
    Q_DECLARE_PRIVATE(Thread)

public:
    Thread();
    Thread(const QString &accountId,
                  const QString &threadId,
                  EventType type,
                  const QStringList &participants,
                  const EventPtr &lastEvent = EventPtr(),
                  int count = 0,
                  int unreadCount = 0);
    ~Thread();

    QString accountId() const;
    QString threadId() const;
    EventType type() const;
    QStringList participants() const;
    EventPtr lastEvent() const;
    int count() const;
    int unreadCount() const;

protected:
    QScopedPointer<ThreadPrivate> d_ptr;
};

}

#endif // HISTORY_THREAD_H
