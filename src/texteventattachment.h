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

#ifndef HISTORY_TEXT_EVENT_ATTACHMENT_H
#define HISTORY_TEXT_EVENT_ATTACHMENT_H

#include <QScopedPointer>
#include <QVariantMap>
#include "types.h"

namespace History
{

class TextEventAttachmentPrivate;
class ItemFactory;

class TextEventAttachment
{
    Q_DECLARE_PRIVATE(TextEventAttachment)
    friend class ItemFactory;

public:
    TextEventAttachment(const QString &accountId,
           const QString &threadId,
           const QString &eventId,
           const QString &attachmentId,
           const QString &contentType,
           const QString &filePath,
           const History::AttachmentFlag &status = History::AttachmentDownloaded);
    virtual ~TextEventAttachment();

    QString accountId() const;
    QString threadId() const;
    QString eventId() const;
    QString attachmentId() const;
    QString contentType() const;
    QString filePath() const;
    History::AttachmentFlag status() const;
    virtual QVariantMap properties() const;

protected:
    QScopedPointer<TextEventAttachmentPrivate> d_ptr;

};

}

#endif // HISTORY_TEXT_EVENT_ATTACHMENT_H
