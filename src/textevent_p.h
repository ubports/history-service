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
                    bool theNewEvent,
                    const QString &theMessage,
                    MessageType theMessageType,
                    MessageFlags theMessageFlags,
                    const QDateTime &theReadTimestamp,
                    const QString &theSubject,
                    const TextEventAttachments &theAttachments);
    ~TextEventPrivate();
    QString message;
    MessageType messageType;
    MessageFlags messageFlags;
    QDateTime readTimestamp;
    QString subject;
    TextEventAttachments attachments;

    EventType type() const;
    QVariantMap properties() const;

    HISTORY_EVENT_DECLARE_CLONE(TextEvent)
};

}

#endif // HISTORY_TEXTEVENT_P_H
