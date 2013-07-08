#include "historyqmlintersectionfilter.h"
#include <HistoryIntersectionFilter>

HistoryQmlIntersectionFilter::HistoryQmlIntersectionFilter(QObject *parent) :
    HistoryQmlCompoundFilter(parent)
{
}

HistoryFilter HistoryQmlIntersectionFilter::filter() const
{
    HistoryIntersectionFilter intersectionFilter;

    Q_FOREACH(HistoryQmlFilter *filter, mFilters) {
        intersectionFilter.append(filter->filter());
    }

    return intersectionFilter;
}
