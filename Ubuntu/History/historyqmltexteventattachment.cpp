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
#include "historyqmltexteventattachment.h"

HistoryQmlTextEventAttachment::HistoryQmlTextEventAttachment(const History::TextEventAttachmentPtr &attachment, QObject *parent) :
    QObject(parent), mAttachment(new History::TextEventAttachment(attachment->accountId(),
                                                                  attachment->threadId(),
                                                                  attachment->eventId(),
                                                                  attachment->attachmentId(),
                                                                  attachment->contentType(),
                                                                  attachment->filePath()))
{
}

QString HistoryQmlTextEventAttachment::accountId() const
{
    return mAttachment->accountId();
}

QString HistoryQmlTextEventAttachment::threadId() const
{
    return mAttachment->threadId();
}

QString HistoryQmlTextEventAttachment::eventId() const
{
    return mAttachment->eventId();
}

QString HistoryQmlTextEventAttachment::attachmentId() const
{
    return mAttachment->attachmentId();
}

QString HistoryQmlTextEventAttachment::contentType() const
{
    return mAttachment->contentType();
}

QString HistoryQmlTextEventAttachment::filePath() const
{
    return mAttachment->filePath();
}

