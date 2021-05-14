/*
 * Copyright (C) 2013-2015 Canonical, Ltd.
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

#ifndef HISTORY_TEXTEVENT_P_H
#define HISTORY_TEXTEVENT_P_H

#include "event_p.h"
#include "types.h"

namespace History
{

class TextEvent;

class TextEventPrivate : public EventPrivate
{
public:
    TextEventPrivate();
    TextEventPrivate(const QString &theAccountId,
                    const QString &theThreadId,
                    const QString &theEventId,
                    const QString &theSender,
                    const QDateTime &theTimestamp,
                    const QDateTime &theSentTime,
                    bool theNewEvent,
                    const QString &theMessage,
                    MessageType theMessageType,
                    MessageStatus theMessageStatus,
                    const QDateTime &theReadTimestamp,
                    const QString &theSubject,
                    InformationType theInformationType,
                    const TextEventAttachments &theAttachments,
                    const Participants &theParticipants);
    ~TextEventPrivate();
    QString message;
    MessageType messageType;
    MessageStatus messageStatus;
    QDateTime readTimestamp;
    QString subject;
    InformationType informationType;
    TextEventAttachments attachments;
    QDateTime sentTime;

    EventType type() const;
    QVariantMap properties() const;

    HISTORY_EVENT_DECLARE_CLONE(TextEvent)
};

}

#endif // HISTORY_TEXTEVENT_P_H
