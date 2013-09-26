/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HISTORY_EVENT_P_H
#define HISTORY_EVENT_P_H

#include <QDateTime>
#include <QString>
#include <QStringList>
#include "types.h"

#define HISTORY_EVENT_DECLARE_CLONE(Class) \
    virtual EventPrivate *clone() { return new Class##Private(*this); }

#define HISTORY_EVENT_DEFINE_COPY(Class, Type) \
    Class::Class(const Event &other) { \
        if (other.type() == Type) { d_ptr = QSharedPointer<Class##Private>(reinterpret_cast<Class##Private*>(EventPrivate::getD(other)->clone())); } \
        else { d_ptr = QSharedPointer<Class##Private>(new Class##Private()); } \
    } \
    Class& Class::operator=(const Event &other) { \
        if (other.type() == Type) { d_ptr = QSharedPointer<Class##Private>(reinterpret_cast<Class##Private*>(EventPrivate::getD(other)->clone())); } \
        return  *this; \
    }

namespace History
{

class EventPrivate
{
public:
    EventPrivate();
    EventPrivate(const QString &theAccountId,
                       const QString &theThreadId,
                       const QString &theEventId,
                       const QString &theSenderId,
                       const QDateTime &theTimestamp,
                       bool theNewEvent,
                       const QStringList &theParticipants);
    virtual ~EventPrivate();

    virtual EventType type() const { return EventTypeNull; }
    virtual QVariantMap properties() const;

    QString accountId;
    QString threadId;
    QString eventId;
    QString senderId;
    QString receiver;
    QDateTime timestamp;
    bool newEvent;
    QStringList participants;

    static const QSharedPointer<EventPrivate>& getD(const Event& other) { return other.d_ptr; }

    HISTORY_EVENT_DECLARE_CLONE(Event)
};

}

#endif // HISTORY_EVENT_P_H
