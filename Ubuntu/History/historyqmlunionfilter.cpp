#include "historyqmlunionfilter.h"
#include <HistoryUnionFilter>

HistoryQmlUnionFilter::HistoryQmlUnionFilter(QObject *parent) :
    HistoryQmlCompoundFilter(parent)
{
}

HistoryFilter HistoryQmlUnionFilter::filter() const
{
    HistoryUnionFilter unionFilter;

    Q_FOREACH(HistoryQmlFilter *filter, mFilters) {
        unionFilter.append(filter->filter());
    }

    return unionFilter;
}
