#ifndef TYPES_H
#define TYPES_H

#include <QSharedPointer>

#define DefineSharedPointer(type) class type; typedef QSharedPointer<type> type ## Ptr;

DefineSharedPointer(HistoryThread)
DefineSharedPointer(HistoryItem)
DefineSharedPointer(HistoryReader)
DefineSharedPointer(HistoryWriter)
DefineSharedPointer(TextItem)
DefineSharedPointer(VoiceItem)

#endif // TYPES_H
