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

#include <QDebug>
#include <QDBusMetaType>

#include "textevent.h"
#include "textevent_p.h"
#include "texteventattachment.h"

Q_DECLARE_METATYPE(QList< QVariantMap >)

namespace History {

// ------------- TextEventPrivate ------------------------------------------------

TextEventPrivate::TextEventPrivate()
{
}

TextEventPrivate::TextEventPrivate(const QString &theAccountId,
                                 const QString &theThreadId,
                                 const QString &theEventId,
                                 const QString &theSender,
                                 const QDateTime &theTimestamp,
                                 bool theNewEvent,
                                 const QString &theMessage,
                                 MessageType theMessageType,
                                 MessageStatus theMessageStatus,
                                 const QDateTime &theReadTimestamp,
                                 const QString &theSubject,
                                 InformationType theInformationType,
                                 const TextEventAttachments &theAttachments, const Participants &theParticipants) :
    EventPrivate(theAccountId, theThreadId, theEventId, theSender, theTimestamp, theNewEvent, theParticipants),
    message(theMessage), messageType(theMessageType), messageStatus(theMessageStatus),
    readTimestamp(theReadTimestamp), subject(theSubject), informationType(theInformationType), attachments(theAttachments)
{
}

TextEventPrivate::~TextEventPrivate()
{
}

EventType TextEventPrivate::type() const
{
    return EventTypeText;
}

QVariantMap TextEventPrivate::properties() const
{
    QVariantMap map = EventPrivate::properties();

    map[FieldMessage] = message;
    map[FieldMessageType] = (int)messageType;
    map[FieldMessageStatus] = (int)messageStatus;
    map[FieldReadTimestamp] = readTimestamp.toString("yyyy-MM-ddTHH:mm:ss.zzz");
    map[FieldSubject] = subject;
    map[FieldInformationType] = informationType;

    QList<QVariantMap> attachmentsMap;
    Q_FOREACH(const TextEventAttachment &attachment, attachments) {
        attachmentsMap << attachment.properties();
    }
    map[FieldAttachments] = QVariant::fromValue(attachmentsMap);

    return map;
}

// ------------- TextEvent -------------------------------------------------------

HISTORY_EVENT_DEFINE_COPY(TextEvent, EventTypeText)

TextEvent::TextEvent()
    : Event(*new TextEventPrivate())
{
}

TextEvent::TextEvent(const QString &accountId,
                   const QString &threadId,
                   const QString &eventId,
                   const QString &sender,
                   const QDateTime &timestamp,
                   bool newEvent,
                   const QString &message,
                   MessageType messageType,
                   MessageStatus messageStatus,
                   const QDateTime &readTimestamp,
                   const QString &subject,
                   InformationType informationType,
                   const TextEventAttachments &attachments,
                   const Participants &participants)
    : Event(*new TextEventPrivate(accountId, threadId, eventId, sender, timestamp, newEvent,
                                  message, messageType, messageStatus, readTimestamp, subject, informationType,
                                  attachments, participants))
{
    qDBusRegisterMetaType<QList<QVariantMap> >();
    qRegisterMetaType<QList<QVariantMap> >();
}

TextEvent::~TextEvent()
{
}

QString TextEvent::message() const
{
    Q_D(const TextEvent);
    return d->message;
}

MessageType TextEvent::messageType() const
{
    Q_D(const TextEvent);
    return d->messageType;
}

MessageStatus TextEvent::messageStatus() const
{
    Q_D(const TextEvent);
    return d->messageStatus;
}

void TextEvent::setMessageStatus(const MessageStatus &value)
{
    Q_D(TextEvent);
    d->messageStatus = value;
}

QDateTime TextEvent::readTimestamp() const
{
    Q_D(const TextEvent);
    return d->readTimestamp;
}

void TextEvent::setReadTimestamp(const QDateTime &value)
{
    Q_D(TextEvent);
    d->readTimestamp = value;
}

QString TextEvent::subject() const
{
    Q_D(const TextEvent);
    return d->subject;
}

InformationType TextEvent::informationType() const
{
    Q_D(const TextEvent);
    return d->informationType;
}

TextEventAttachments TextEvent::attachments() const
{
    Q_D(const TextEvent);
    return d->attachments;
}

Event TextEvent::fromProperties(const QVariantMap &properties)
{
    Event event;
    if (properties.isEmpty()) {
        return event;
    }

    QString accountId = properties[FieldAccountId].toString();
    QString threadId = properties[FieldThreadId].toString();
    QString eventId = properties[FieldEventId].toString();
    QString senderId = properties[FieldSenderId].toString();
    QDateTime timestamp = QDateTime::fromString(properties[FieldTimestamp].toString(), Qt::ISODate);
    bool newEvent = properties[FieldNewEvent].toBool();
    Participants participants = Participants::fromVariant(properties[FieldParticipants]);
    QString message = properties[FieldMessage].toString();
    QString subject = properties[FieldSubject].toString();
    InformationType informationType = (InformationType) properties[FieldInformationType].toInt();
    MessageType messageType = (MessageType) properties[FieldMessageType].toInt();
    MessageStatus messageStatus = (MessageStatus) properties[FieldMessageStatus].toInt();
    QDateTime readTimestamp = QDateTime::fromString(properties[FieldReadTimestamp].toString(), Qt::ISODate);

    // read the attachments
    QList<QVariantMap> attachmentProperties = qdbus_cast<QList<QVariantMap> >(properties[FieldAttachments]);
    // dbus_cast fails if the map was generated by a qml app, so we demarshal it by hand
    if (attachmentProperties.isEmpty()) {
        QVariantList attachmentList = properties[FieldAttachments].toList();
        Q_FOREACH(const QVariant &attachmentMap, attachmentList) {
            attachmentProperties << attachmentMap.toMap();
        }
    }
    TextEventAttachments attachments;
    Q_FOREACH(const QVariantMap &map, attachmentProperties) {
        TextEventAttachment attachment = TextEventAttachment::fromProperties(map);
        if (!attachment.isNull()) {
            attachments << attachment;
        }
    }

    // and finally create the event
    event = TextEvent(accountId, threadId, eventId, senderId, timestamp, newEvent,
                      message, messageType, messageStatus, readTimestamp, subject, informationType, attachments, participants);
    return event;
}

}
