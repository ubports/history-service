/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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
    MatchPhoneNumber = 0x08,
    MatchNotEquals = 0x10
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
    MessageStatusPending, // pending attachment download
    MessageStatusDraft
};

enum MessageType
{
    MessageTypeText = 0,
    MessageTypeMultiPart = 1,
    MessageTypeInformation = 2
};

enum InformationType
{
    InformationTypeNone = 0,
    InformationTypeSimChange = 1,
    InformationTypeText = 2,
    InformationTypeSelfJoined = 3,
    InformationTypeJoined = 4,
    InformationTypeTitleChanged = 5,
    InformationTypeInvitationSent = 6,
    InformationTypeLeaving = 7,
    InformationTypeSelfLeaving = 8,
    InformationTypeAdminGranted = 9,
    InformationTypeAdminRemoved = 10,
    InformationTypeSelfAdminGranted = 11,
    InformationTypeSelfAdminRemoved = 12,
    InformationTypeSelfKicked = 13,
    InformationTypeGroupGone = 14
};

enum ChatType
{
    ChatTypeNone = 0,
    ChatTypeContact = 1,
    ChatTypeRoom = 2
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

enum ParticipantState
{
    ParticipantStateRegular = 0,
    ParticipantStateLocalPending = 1,
    ParticipantStateRemotePending = 2
};

enum ParticipantRoles
{
    ParticipantRoleNone = 0,
    ParticipantRoleMember = 1,
    ParticipantRoleAdmin = 2
};

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
static const char* FieldParticipantIds = "participantIds";
static const char* FieldCount = "count";
static const char* FieldUnreadCount = "unreadCount";
static const char* FieldSenderId = "senderId";
static const char* FieldTimestamp = "timestamp";
static const char* FieldDate = "date";
static const char* FieldNewEvent = "newEvent";
static const char* FieldChatType = "chatType";
static const char* FieldChatRoomInfo = "chatRoomInfo";
static const char* FieldChatRoomJoined = "joined";
static const char* FieldChatRoomSelfRoles = "selfRoles";

// Chat Room Info Fields
static const char* FieldChatRoomName = "roomName";
static const char* FieldChatRoomServer = "server";
static const char* FieldChatRoomCreator = "creator";
static const char* FieldChatRoomCreationTimestamp = "creationTimestamp";
static const char* FieldChatRoomAnonymous = "anonymous";
static const char* FieldChatRoomInviteOnly = "inviteOnly";
static const char* FieldChatRoomParticipantLimit = "participantLimit";
static const char* FieldChatRoomModerated = "moderated";
static const char* FieldChatRoomTitle = "title";
static const char* FieldChatRoomDescription = "description";
static const char* FieldChatRoomPersistent = "persistent";
static const char* FieldChatRoomPrivate = "private";
static const char* FieldChatRoomPasswordProtected = "passwordProtected";
static const char* FieldChatRoomPassword = "password";
static const char* FieldChatRoomPasswordHint = "passwordHint";
static const char* FieldChatRoomCanUpdateConfiguration = "canUpdateConfiguration";

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
static const char* FieldInformationType = "informationType";
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
static const char* FieldParticipantState = "state";
static const char* FieldParticipantRoles = "roles";

}

#endif // TYPES_H
