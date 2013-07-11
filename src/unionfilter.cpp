#include "unionfilter.h"
#include "unionfilter_p.h"
#include <QStringList>

namespace History
{

UnionFilterPrivate::UnionFilterPrivate()
{
}

UnionFilterPrivate::~UnionFilterPrivate()
{
}

UnionFilter::UnionFilter()
    : Filter(*new UnionFilterPrivate())
{
}

UnionFilter::~UnionFilter()
{
}

void UnionFilter::setFilters(const QList<FilterPtr> &filters)
{
    Q_D(UnionFilter);
    d->filters = filters;
}

void UnionFilter::prepend(const FilterPtr &filter)
{
    Q_D(UnionFilter);
    d->filters.prepend(filter);
}

void UnionFilter::append(const FilterPtr &filter)
{
    Q_D(UnionFilter);
    d->filters.append(filter);
}

void UnionFilter::clear()
{
    Q_D(UnionFilter);
    d->filters.clear();
}

QList<FilterPtr> UnionFilter::filters() const
{
    Q_D(const UnionFilter);
    return d->filters;
}

QString UnionFilter::toString() const
{
    Q_D(const UnionFilter);

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

    return output.join(" OR ");
}

}
