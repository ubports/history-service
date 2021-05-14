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

#ifndef HISTORY_TEXTEVENT_H
#define HISTORY_TEXTEVENT_H

#include "event.h"
#include "texteventattachment.h"

namespace History
{

class TextEventPrivate;
class ItemFactory;

class TextEvent : public Event
{
    Q_DECLARE_PRIVATE(TextEvent)
    friend class ItemFactory;

public:
    explicit TextEvent();
    TextEvent(const QString &accountId,
              const QString &threadId,
              const QString &eventId,
              const QString &sender,
              const QDateTime &timestamp,
              bool newEvent,
              const QString &message,
              MessageType messageType,
              MessageStatus messageStatus = MessageStatusUnknown,
              const QDateTime &readTimestamp = QDateTime(),
              const QString &subject = QString(),
              InformationType informationType = InformationTypeNone,
              const TextEventAttachments &attachments = TextEventAttachments(),
              const Participants &participants = Participants());

    TextEvent(const QString &accountId,
              const QString &threadId,
              const QString &eventId,
              const QString &sender,
              const QDateTime &timestamp,
              const QDateTime &sentTime,
              bool newEvent,
              const QString &message,
              MessageType messageType,
              MessageStatus messageStatus = MessageStatusUnknown,
              const QDateTime &readTimestamp = QDateTime(),
              const QString &subject = QString(),
              InformationType informationType = InformationTypeNone,
              const TextEventAttachments &attachments = TextEventAttachments(),
              const Participants &participants = Participants());

    ~TextEvent();

    // copy related members
    TextEvent(const Event &other);
    TextEvent& operator=(const Event &other);

    QString message() const;
    MessageType messageType() const;
    MessageStatus messageStatus() const;
    void setMessageStatus(const MessageStatus &value);
    QDateTime sentTime() const;
    void setSentTime(const QDateTime &value);
    QDateTime readTimestamp() const;
    void setReadTimestamp(const QDateTime &value);
    QString subject() const;
    InformationType informationType() const;
    TextEventAttachments attachments() const;

    static Event fromProperties(const QVariantMap &properties);
};

}

#endif // HISTORY_TEXTEVENT_H
