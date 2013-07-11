#ifndef TYPES_H
#define TYPES_H

#include <QSharedPointer>

#define DefineSharedPointer(type) class type; typedef QSharedPointer<type> type ## Ptr;

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
    Pending,
    Delivered
};

Q_DECLARE_FLAGS(MessageFlags, MessageFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(MessageFlags)

enum MessageType
{
    TextMessage,
    MultiPartMessage
};

}

#endif // TYPES_H
