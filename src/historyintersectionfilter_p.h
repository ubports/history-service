#ifndef HISTORYINTERSECTIONFILTER_P_H
#define HISTORYINTERSECTIONFILTER_P_H

#include "historyfilter_p.h"

class HistoryIntersectionFilter;

class HistoryIntersectionFilterPrivate : public HistoryFilterPrivate
{

public:
    HistoryIntersectionFilterPrivate();
    ~HistoryIntersectionFilterPrivate();

    QList<HistoryFilterPtr> filters;
};

#endif // HISTORYINTERSECTIONFILTER_P_H
