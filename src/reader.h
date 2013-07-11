#ifndef HISTORY_READER_H
#define HISTORY_READER_H

#include <QObject>
#include "types.h"

namespace History
{

class Reader : public QObject
{
    Q_OBJECT
public:
    explicit Reader(QObject *parent = 0) : QObject(parent) {}
    virtual ~Reader() {}

    virtual ThreadViewPtr queryThreads(EventType type,
                                       const SortPtr &sort = SortPtr(),
                                       const FilterPtr &filter = FilterPtr()) = 0;
    virtual EventViewPtr queryEvents(EventType type,
                                     const SortPtr &sort = SortPtr(),
                                     const FilterPtr &filter = FilterPtr()) = 0;
};

}
#endif // HISTORY_READER_H
