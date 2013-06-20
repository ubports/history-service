#ifndef TYPES_H
#define TYPES_H

#include <QSharedPointer>

class HistoryThread;
class HistoryItem;

typedef QSharedPointer<HistoryThread> HistoryThreadPtr;
typedef QSharedPointer<HistoryItem> HistoryItemPtr;

#endif // TYPES_H
