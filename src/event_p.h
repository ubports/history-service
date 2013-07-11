#ifndef HISTORY_EVENT_P_H
#define HISTORY_EVENT_P_H

#include <QDateTime>
#include <QString>
#include "types.h"

namespace History
{

class EventPrivate
{
public:
    EventPrivate();
    EventPrivate(const QString &theAccountId,
                       const QString &theThreadId,
                       const QString &theItemId,
                       const QString &theSender,
                       const QDateTime &theTimestamp,
                       bool theNewItem);
    virtual ~EventPrivate();


    QString accountId;
    QString threadId;
    QString eventId;
    QString sender;
    QString receiver;
    QDateTime timestamp;
    bool newEvent;
};

}

#endif // HISTORY_EVENT_P_H
