#include "threadview.h"
#include "threadview_p.h"
#include "filter.h"
#include "thread.h"

namespace History
{

// ------------- ThreadViewPrivate ------------------------------------------------

ThreadViewPrivate::ThreadViewPrivate(History::EventType theType,
                                     const History::SortPtr &theSort,
                                     const History::FilterPtr &theFilter)
    : type(theType), sort(theSort), filter(theFilter)
{
}

Threads ThreadViewPrivate::filteredThreads(const Threads &threads)
{
    // if the filter is null, return all threads
    if (filter.isNull()) {
        return threads;
    }

    Threads filtered;
    Q_FOREACH(const ThreadPtr &thread, threads) {
        if (filter->match(thread->properties())) {
            filtered << thread;
        }
    }

    return filtered;
}

void ThreadViewPrivate::_d_threadsAdded(const History::Threads &threads)
{
    Q_Q(ThreadView);

    Threads filtered = filteredThreads(threads);
    if (!filtered.isEmpty()) {
        Q_EMIT q->threadsAdded(filtered);
    }
}

void ThreadViewPrivate::_d_threadsModified(const Threads &threads)
{
    Q_Q(ThreadView);

    Threads filtered = filteredThreads(threads);
    if (!filtered.isEmpty()) {
        Q_EMIT q->threadsModified(filtered);
    }
}

void ThreadViewPrivate::_d_threadsRemoved(const Threads &threads)
{
    Q_Q(ThreadView);

    Threads filtered = filteredThreads(threads);
    if (!filtered.isEmpty()) {
        Q_EMIT q->threadsRemoved(filtered);
    }
}

// ------------- ThreadView -------------------------------------------------------

ThreadView::ThreadView(History::EventType type,
                       const History::SortPtr &sort,
                       const History::FilterPtr &filter)
    : d_ptr(new ThreadViewPrivate(type, sort, filter))
{
    d_ptr->q_ptr = this;
}

ThreadView::~ThreadView()
{
}

}

#include "moc_threadview.cpp"
