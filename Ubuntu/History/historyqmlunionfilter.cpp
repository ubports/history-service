#include "historyqmlunionfilter.h"
#include "unionfilter.h"

HistoryQmlUnionFilter::HistoryQmlUnionFilter(QObject *parent) :
    HistoryQmlCompoundFilter(parent), mFilter(new History::UnionFilter())
{
}

History::FilterPtr HistoryQmlUnionFilter::filter() const
{
    mFilter->clear();
    Q_FOREACH(HistoryQmlFilter *filter, mFilters) {
        mFilter->append(filter->filter());
    }

    return mFilter;
}
