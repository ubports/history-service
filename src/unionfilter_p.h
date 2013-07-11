#ifndef HISTORY_UNIONFILTER_P_H
#define HISTORY_UNIONFILTER_P_H

#include "filter_p.h"

namespace History
{

class UnionFilter;

class UnionFilterPrivate : public FilterPrivate
{

public:
    UnionFilterPrivate();
    ~UnionFilterPrivate();

    QList<FilterPtr> filters;
};

}

#endif // HISTORY_UNIONFILTER_P_H
