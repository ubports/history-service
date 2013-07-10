#ifndef HISTORYQMLINTERSECTIONFILTER_H
#define HISTORYQMLINTERSECTIONFILTER_H

#include "historyqmlfilter.h"
#include <Types>

class HistoryQmlIntersectionFilter : public HistoryQmlCompoundFilter
{
    Q_OBJECT
public:
    explicit HistoryQmlIntersectionFilter(QObject *parent = 0);

    HistoryFilterPtr filter() const;

private:
    HistoryIntersectionFilterPtr mFilter;
};

#endif // HISTORYQMLINTERSECTIONFILTER_H
