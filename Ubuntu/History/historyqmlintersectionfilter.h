#ifndef HISTORYQMLINTERSECTIONFILTER_H
#define HISTORYQMLINTERSECTIONFILTER_H

#include "historyqmlfilter.h"
#include "types.h"

class HistoryQmlIntersectionFilter : public HistoryQmlCompoundFilter
{
    Q_OBJECT
public:
    explicit HistoryQmlIntersectionFilter(QObject *parent = 0);

    History::FilterPtr filter() const;

private:
    History::IntersectionFilterPtr mFilter;
};

#endif // HISTORYQMLINTERSECTIONFILTER_H
