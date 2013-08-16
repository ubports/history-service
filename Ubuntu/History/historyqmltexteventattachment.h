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

#ifndef HISTORYQMLTEXTEVENTATTACHMENT_H
#define HISTORYQMLTEXTEVENTATTACHMENT_H

#include <qqml.h>
#include <QObject>
#include "historyqmltexteventattachment.h"
#include "types.h"

class HistoryQmlTextEventAttachment : public QObject
{
    Q_OBJECT
    Q_ENUMS(AttachmentFlag)
    Q_PROPERTY(QString accountId READ accountId)
    Q_PROPERTY(QString threadId READ threadId)
    Q_PROPERTY(QString eventId READ eventId)
    Q_PROPERTY(QString attachmentId READ attachmentId)
    Q_PROPERTY(QString contentType READ contentType)
    Q_PROPERTY(QString filePath READ filePath)
    Q_PROPERTY(int status READ status)
public:
    enum AttachmentFlag
    {
        AttachmentDownloaded = History::AttachmentDownloaded,
        AttachmentPending = History::AttachmentPending,
        AttachmentError = History::AttachmentError
    };
    explicit HistoryQmlTextEventAttachment(const History::TextEventAttachmentPtr &attachment, QObject *parent = 0);

    QString accountId() const;
    QString threadId() const;
    QString eventId() const;
    QString attachmentId() const;
    QString contentType() const;
    QString filePath() const;
    int status() const;

protected:
    History::TextEventAttachmentPtr mAttachment;
};

#endif // HISTORYQMLTEXTEVENTATTACHMENT_H
