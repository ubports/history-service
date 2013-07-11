#ifndef HISTORY_INTERSECTIONFILTER_H
#define HISTORY_INTERSECTIONFILTER_H

#include "filter.h"
#include "types.h"

namespace History
{

class IntersectionFilterPrivate;

// AND filter
class IntersectionFilter : public Filter
{
    Q_DECLARE_PRIVATE(IntersectionFilter)
public:
    IntersectionFilter();
    ~IntersectionFilter();

    void setFilters(const QList<FilterPtr> &filters);
    void prepend(const FilterPtr &filter);
    void append(const FilterPtr &filter);
    void clear();

    QList<FilterPtr> filters() const;
    QString toString() const;
};

}

#endif
