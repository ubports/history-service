/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "texteventattachment.h"
#include "texteventattachment_p.h"

namespace History
{

// ------------- TextEventAttachmentPrivate ------------------------------------------------

TextEventAttachmentPrivate::TextEventAttachmentPrivate(const QString &theAccountId,
                                           const QString &theThreadId,
                                           const QString &theEventId,
                                           const QString &theAttachmentId,
                                           const QString &theContentType,
                                           const QString &theFilePath,
                                           const AttachmentFlags &theStatus) :
    accountId(theAccountId), threadId(theThreadId), eventId(theEventId), attachmentId(theAttachmentId),
    contentType(theContentType), filePath(theFilePath), status(theStatus)
{
}

TextEventAttachmentPrivate::~TextEventAttachmentPrivate()
{
}

// ------------- TextEventAttachment ------------------------------------------------------

/*!
 * \class TextEventAttachment
 *
 * \brief The TextEventAttachment class provides a way to store a single attachment
 *  belonging to a text event.
 *
 */
TextEventAttachment::TextEventAttachment(const QString &accountId,
               const QString &threadId, const QString &eventId,
               const QString &attachmentId,
               const QString &contentType,
               const QString &filePath,
               const AttachmentFlags &status)
: d_ptr(new TextEventAttachmentPrivate(accountId, threadId, eventId, attachmentId, contentType, filePath, status))
{
}

TextEventAttachment::~TextEventAttachment()
{
}


/*!
 * \brief Returns the account ID this attachment belongs to.
 */
QString TextEventAttachment::accountId() const
{
    Q_D(const TextEventAttachment);
    return d->accountId;
}

/*!
 * \brief Returns the thread ID this attachment belongs to.
 */
QString TextEventAttachment::threadId() const
{
    Q_D(const TextEventAttachment);
    return d->threadId;
}

/*!
 * \brief Returns the event ID this attachment belongs to.
 */
QString TextEventAttachment::eventId() const
{
    Q_D(const TextEventAttachment);
    return d->eventId;
}

/*!
 * \brief Returns the attachment ID
 */
QString TextEventAttachment::attachmentId() const
{
    Q_D(const TextEventAttachment);
    return d->attachmentId;
}

/*!
 * \brief Returns the content type of this attachment
 */
QString TextEventAttachment::contentType() const
{
    Q_D(const TextEventAttachment);
    return d->contentType;
}

/*!
 * \brief Returns the file path of this attachment
 */
QString TextEventAttachment::filePath() const
{
    Q_D(const TextEventAttachment);
    return d->filePath;
}

/*!
 * \brief Returns the status of this attachment
 */
AttachmentFlags TextEventAttachment::status() const
{
    Q_D(const TextEventAttachment);
    return d->status;
}

QVariantMap TextEventAttachment::properties() const
{
    Q_D(const TextEventAttachment);

    QVariantMap map;
    map[FieldAccountId] = d->accountId;
    map[FieldThreadId] = d->threadId;
    map[FieldEventId] = d->eventId;
    map[FieldAttachmentId] = d->attachmentId;
    map[FieldContentType] = d->contentType;
    map[FieldFilePath] = d->filePath;
    map[FieldStatus] = (int)d->status;

    return map;
}

TextEventAttachmentPtr TextEventAttachment::fromProperties(const QVariantMap &properties)
{
    TextEventAttachmentPtr attachment;
    if (properties.isEmpty()) {
        return attachment;
    }

    attachment = TextEventAttachmentPtr(new TextEventAttachment(properties[FieldAccountId].toString(),
                                                                properties[FieldThreadId].toString(),
                                                                properties[FieldEventId].toString(),
                                                                properties[FieldAttachmentId].toString(),
                                                                properties[FieldContentType].toString(),
                                                                properties[FieldFilePath].toString(),
                                                                (History::AttachmentFlags)properties[FieldStatus].toInt()));
    return attachment;
}

}
