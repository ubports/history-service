#ifndef HISTORY_INTERSECTIONFILTER_P_H
#define HISTORY_INTERSECTIONFILTER_P_H

#include "filter_p.h"

namespace History
{

class IntersectionFilter;

class IntersectionFilterPrivate : public FilterPrivate
{

public:
    IntersectionFilterPrivate();
    ~IntersectionFilterPrivate();

    QList<FilterPtr> filters;
};

}

#endif // HISTORY_INTERSECTIONFILTER_P_H
