#ifndef TYPES_H
#define TYPES_H

#include <QSharedPointer>

#define DefineSharedPointer(type) class type; typedef QSharedPointer<type> type ## Ptr;

DefineSharedPointer(HistoryItem)
DefineSharedPointer(HistoryItemView)
DefineSharedPointer(HistoryReader)
DefineSharedPointer(HistoryThread)
DefineSharedPointer(HistoryThreadView)
DefineSharedPointer(HistoryWriter)
DefineSharedPointer(TextItem)
DefineSharedPointer(VoiceItem)

// filters and sorting
DefineSharedPointer(HistorySort)
DefineSharedPointer(HistoryFilter)
DefineSharedPointer(HistoryIntersectionFilter)
DefineSharedPointer(HistoryUnionFilter)

#endif // TYPES_H
