#include "historyqmlunionfilter.h"
#include <HistoryUnionFilter>

HistoryQmlUnionFilter::HistoryQmlUnionFilter(QObject *parent) :
    HistoryQmlCompoundFilter(parent), mFilter(new HistoryUnionFilter())
{
}

HistoryFilterPtr HistoryQmlUnionFilter::filter() const
{
    mFilter->clear();
    Q_FOREACH(HistoryQmlFilter *filter, mFilters) {
        mFilter->append(filter->filter());
    }

    return mFilter;
}
