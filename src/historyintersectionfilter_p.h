#ifndef HISTORYINTERSECTIONFILTER_P_H
#define HISTORYINTERSECTIONFILTER_P_H

#include "historyfilter_p.h"

class HistoryIntersectionFilter;

class HistoryIntersectionFilterPrivate : public HistoryFilterPrivate
{

public:
    HistoryIntersectionFilterPrivate();
    ~HistoryIntersectionFilterPrivate();

    QList<HistoryFilter> filters;
};

#endif // HISTORYINTERSECTIONFILTER_P_H
