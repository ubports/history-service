#ifndef THREADVIEW_P_H
#define THREADVIEW_P_H

#include "types.h"

namespace History
{
    class ThreadView;

    class ThreadViewPrivate
    {
        Q_DECLARE_PUBLIC(ThreadView)

    public:
        ThreadViewPrivate(History::EventType theType,
                          const History::SortPtr &theSort,
                          const History::FilterPtr &theFilter);
        EventType type;
        SortPtr sort;
        FilterPtr filter;

        Threads filteredThreads(const Threads &threads);

        // private slots
        void _d_threadsAdded(const History::Threads &threads);
        void _d_threadsModified(const History::Threads &threads);
        void _d_threadsRemoved(const History::Threads &threads);

        ThreadView *q_ptr;
    };
}

#endif // THREADVIEW_P_H
