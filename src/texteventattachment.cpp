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
                                           const AttachmentFlag &theStatus) :
    accountId(theAccountId), threadId(theThreadId), eventId(theEventId), attachmentId(theAttachmentId),
    contentType(theContentType), filePath(theFilePath), status(theStatus)
{
}

TextEventAttachmentPrivate::~TextEventAttachmentPrivate()
{
}

// ------------- TextEventAttachment ------------------------------------------------------

TextEventAttachment::TextEventAttachment(const QString &accountId,
               const QString &threadId, const QString &eventId,
               const QString &attachmentId,
               const QString &contentType,
               const QString &filePath,
               const AttachmentFlag &status)
: d_ptr(new TextEventAttachmentPrivate(accountId, threadId, eventId, attachmentId, contentType, filePath, status))
{
}

TextEventAttachment::~TextEventAttachment()
{
}

QString TextEventAttachment::accountId() const
{
    Q_D(const TextEventAttachment);
    return d->accountId;
}

QString TextEventAttachment::threadId() const
{
    Q_D(const TextEventAttachment);
    return d->threadId;
}

QString TextEventAttachment::eventId() const
{
    Q_D(const TextEventAttachment);
    return d->eventId;
}

QString TextEventAttachment::attachmentId() const
{
    Q_D(const TextEventAttachment);
    return d->attachmentId;
}

QString TextEventAttachment::contentType() const
{
    Q_D(const TextEventAttachment);
    return d->contentType;
}

QString TextEventAttachment::filePath() const
{
    Q_D(const TextEventAttachment);
    return d->filePath;
}

AttachmentFlag TextEventAttachment::status() const
{
    Q_D(const TextEventAttachment);
    return d->status;
}

QVariantMap TextEventAttachment::properties() const
{
    Q_D(const TextEventAttachment);

    QVariantMap map;
    map["accountId"] = d->accountId;
    map["threadId"] = d->threadId;
    map["eventId"] = d->eventId;
    map["attachmentId"] = d->attachmentId;
    map["contentType"] = d->contentType;
    map["filePath"] = d->filePath;
    map["status"] = d->status;

    return map;
}

}
