#include "historyqmlintersectionfilter.h"
#include "intersectionfilter.h"

HistoryQmlIntersectionFilter::HistoryQmlIntersectionFilter(QObject *parent) :
    HistoryQmlCompoundFilter(parent), mFilter(new History::IntersectionFilter())
{
}

History::FilterPtr HistoryQmlIntersectionFilter::filter() const
{
    mFilter->clear();
    Q_FOREACH(HistoryQmlFilter *filter, mFilters) {
        mFilter->append(filter->filter());
    }

    return mFilter;
}
