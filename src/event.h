#ifndef HISTORY_EVENT_H
#define HISTORY_EVENT_H

#include <QDateTime>
#include <QScopedPointer>
#include <QString>
#include "types.h"

namespace History
{

class EventPrivate;

class Event
{
    Q_DECLARE_PRIVATE(Event)

public:
    virtual ~Event();

    QString accountId() const;
    QString threadId() const;
    QString eventId() const;
    QString sender() const;
    QDateTime timestamp() const;
    bool newEvent() const;
    virtual EventType type() const = 0;

protected:
    Event(EventPrivate &p);
    QScopedPointer<EventPrivate> d_ptr;
};

}

#endif
