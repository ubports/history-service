#ifndef HISTORY_UNIONFILTER_H
#define HISTORY_UNIONFILTER_H

#include "filter.h"
#include "types.h"

namespace History
{

class UnionFilterPrivate;

// OR filter
class UnionFilter : public Filter
{
    Q_DECLARE_PRIVATE(UnionFilter)
public:
    UnionFilter();
    ~UnionFilter();

    void setFilters(const QList<FilterPtr> &filters);
    void prepend(const FilterPtr &filter);
    void append(const FilterPtr &filter);
    void clear();

    QList<FilterPtr> filters() const;
    QString toString() const;
};

}

#endif
