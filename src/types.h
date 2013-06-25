#ifndef TYPES_H
#define TYPES_H

#include <QSharedPointer>

class HistoryThread;
class HistoryItem;
class HistoryReader;
class HistoryWriter;

typedef QSharedPointer<HistoryThread> HistoryThreadPtr;
typedef QSharedPointer<HistoryItem> HistoryItemPtr;
typedef QSharedPointer<HistoryReader> HistoryReaderPtr;
typedef QSharedPointer<HistoryWriter> HistoryWriterPtr;

#endif // TYPES_H
