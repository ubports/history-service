#include "historyqmlintersectionfilter.h"
#include <HistoryIntersectionFilter>

HistoryQmlIntersectionFilter::HistoryQmlIntersectionFilter(QObject *parent) :
    HistoryQmlCompoundFilter(parent), mFilter(new HistoryIntersectionFilter())
{
}

HistoryFilterPtr HistoryQmlIntersectionFilter::filter() const
{
    mFilter->clear();
    Q_FOREACH(HistoryQmlFilter *filter, mFilters) {
        mFilter->append(filter->filter());
    }

    return mFilter;
}
