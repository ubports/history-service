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

#ifndef TYPES_H
#define TYPES_H

#include <QSharedPointer>
#include <QFlags>

#define DefineSharedPointer(type) class type; typedef QSharedPointer<type> type##Ptr; typedef QWeakPointer<type> type##WeakPtr; typedef QList<type##Ptr> type##s;

namespace History
{

DefineSharedPointer(EventView)
DefineSharedPointer(Plugin)
DefineSharedPointer(ThreadView)

// enums
enum EventType {
    EventTypeText,
    EventTypeVoice,
    EventTypeNull
};

enum FilterType {
    FilterTypeStandard,
    FilterTypeIntersection,
    FilterTypeUnion
};

enum MatchFlag {
    MatchCaseSensitive = 0x01,
    MatchCaseInsensitive = 0x02,
    MatchContains = 0x04,
    MatchPhoneNumber = 0x08
};

Q_DECLARE_FLAGS(MatchFlags, MatchFlag)

enum MessageStatus
{
    MessageStatusUnknown,
    MessageStatusDelivered,
    MessageStatusTemporarilyFailed,
    MessageStatusPermanentlyFailed,
    MessageStatusAccepted,
    MessageStatusRead,
    MessageStatusDeleted,
    MessageStatusPending // pending attachment download
};

enum MessageType
{
    MessageTypeText = 0,
    MessageTypeMultiPart = 1,
    MessageTypeInformation = 2
};

// FIXME (boiko): I think this needs to be changed to a simple enum and not flags,
// as the statuses are mutually exclusive
enum AttachmentFlag
{
    AttachmentDownloaded = 0x01,
    AttachmentPending = 0x02,
    AttachmentError = 0x04
};

Q_DECLARE_FLAGS(AttachmentFlags, AttachmentFlag)

// Event writing results
enum EventWriteResult {
    EventWriteCreated,
    EventWriteModified,
    EventWriteError
};

// dbus service, object path and interface
static const char* DBusService = "com.canonical.HistoryService";
static const char* DBusObjectPath = "/com/canonical/HistoryService";
static const char* DBusInterface = "com.canonical.HistoryService";
static const char* ThreadViewInterface = "com.canonical.HistoryService.ThreadView";
static const char* EventViewInterface = "com.canonical.HistoryService.EventView";

// fields
static const char* FieldAccountId = "accountId";
static const char* FieldThreadId = "threadId";
static const char* FieldEventId = "eventId";
static const char* FieldType = "type";
static const char* FieldParticipants = "participants";
static const char* FieldCount = "count";
static const char* FieldUnreadCount = "unreadCount";
static const char* FieldSenderId = "senderId";
static const char* FieldTimestamp = "timestamp";
static const char* FieldDate = "date";
static const char* FieldNewEvent = "newEvent";

// thread fields
static const char* FieldLastEventId = "lastEventId";
static const char* FieldLastEventTimestamp = "lastEventTimestamp";
static const char* FieldGroupedThreads = "groupedThreads";

// text event fields
static const char* FieldMessage = "message";
static const char* FieldMessageType = "messageType";
static const char* FieldMessageStatus = "messageStatus";
static const char* FieldReadTimestamp = "readTimestamp";
static const char* FieldSubject = "subject";
static const char* FieldAttachments = "attachments";

// text attachment fields

static const char* FieldAttachmentId = "attachmentId";
static const char* FieldContentType = "contentType";
static const char* FieldFilePath = "filePath";
static const char* FieldStatus = "status";

// voice event fields
static const char* FieldMissed = "missed";
static const char* FieldDuration = "duration";
static const char* FieldRemoteParticipant = "remoteParticipant";

// sort stuff
static const char* FieldSortField = "sortField";
static const char* FieldSortOrder = "sortOrder";
static const char* FieldCaseSensitivity = "caseSensitivity";

// filter stuff
static const char* FieldFilterType = "filterType";
static const char* FieldFilterProperty = "filterProperty";
static const char* FieldFilterValue = "filterValue";
static const char* FieldMatchFlags = "matchFlags";
static const char* FieldFilters = "filters";
static const char* FieldGroupingProperty = "groupingProperty";

// contact matching stuff
static const char* FieldContactId = "contactId";
static const char* FieldAlias = "alias";
static const char* FieldAvatar = "avatar";
static const char* FieldIdentifier = "identifier";
static const char* FieldDetailProperties = "detailProperties";

}

#endif // TYPES_H
