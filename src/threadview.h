#ifndef HISTORY_THREADVIEW_H
#define HISTORY_THREADVIEW_H

#include "types.h"

namespace History
{

class ThreadView
{
public:
    ThreadView() {}
    virtual ~ThreadView() {}

    virtual QList<History::ThreadPtr> nextPage() = 0;
    virtual bool isValid() const = 0;
};

}

#endif // HISTORY_THREADVIEW_H
