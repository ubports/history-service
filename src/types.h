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

#define DefineSharedPointer(type) class type; typedef QSharedPointer<type> type##Ptr; typedef QWeakPointer<type> type##WeakPtr; typedef QList<type##Ptr> type##s;

namespace History
{

DefineSharedPointer(Event)
DefineSharedPointer(EventView)
DefineSharedPointer(Filter)
DefineSharedPointer(IntersectionFilter)
DefineSharedPointer(Plugin)
DefineSharedPointer(Reader)
DefineSharedPointer(Sort)
DefineSharedPointer(TextEvent)
DefineSharedPointer(Thread)
DefineSharedPointer(ThreadView)
DefineSharedPointer(UnionFilter)
DefineSharedPointer(VoiceEvent)
DefineSharedPointer(Writer)


// enums
enum EventType {
    EventTypeText,
    EventTypeVoice
};


enum MatchFlag {
    MatchCaseSensitive,
    MatchCaseInsensitive,
    MatchContains,
    MatchPhoneNumber
};

Q_DECLARE_FLAGS(MatchFlags, MatchFlag)

enum MessageFlag
{
    MessageFlagPending,
    MessageFlagDelivered
};

Q_DECLARE_FLAGS(MessageFlags, MessageFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(MessageFlags)

enum MessageType
{
    MessageTypeText,
    MessageTypeMultiParty
};

}

#endif // TYPES_H
