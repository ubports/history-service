#include "intersectionfilter.h"
#include "intersectionfilter_p.h"
#include <QStringList>

namespace History
{

IntersectionFilterPrivate::IntersectionFilterPrivate()
{
}

IntersectionFilterPrivate::~IntersectionFilterPrivate()
{
}

IntersectionFilter::IntersectionFilter()
    : Filter(*new IntersectionFilterPrivate())
{
}

IntersectionFilter::~IntersectionFilter()
{
}

void IntersectionFilter::setFilters(const QList<FilterPtr> &filters)
{
    Q_D(IntersectionFilter);
    d->filters = filters;
}

void IntersectionFilter::prepend(const FilterPtr &filter)
{
    Q_D(IntersectionFilter);
    d->filters.prepend(filter);
}

void IntersectionFilter::append(const FilterPtr &filter)
{
    Q_D(IntersectionFilter);
    d->filters.append(filter);
}

void IntersectionFilter::clear()
{
    Q_D(IntersectionFilter);
    d->filters.clear();
}

QList<FilterPtr> IntersectionFilter::filters() const
{
    Q_D(const IntersectionFilter);
    return d->filters;
}

QString IntersectionFilter::toString() const
{
    Q_D(const IntersectionFilter);

    if (d->filters.isEmpty()) {
        return QString::null;
    } else if (d->filters.count() == 1) {
        return d->filters.first()->toString();
    }

    QStringList output;
    // wrap each filter string around parenthesis
    Q_FOREACH(const FilterPtr &filter, d->filters) {
        output << QString("(%1)").arg(filter->toString());
    }

    return output.join(" AND ");
}

}
