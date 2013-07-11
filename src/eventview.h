#ifndef HISTORY_EVENTVIEW_H
#define HISTORY_EVENTVIEW_H

#include "types.h"

namespace History
{

class EventView
{
public:
    EventView() {}
    virtual ~EventView() {}

    virtual QList<History::EventPtr> nextPage() = 0;
    virtual bool isValid() const = 0;
};

}

#endif // HISTORY_EVENTVIEW_H
