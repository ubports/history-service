#include "historyintersectionfilter.h"
#include "historyintersectionfilter_p.h"
#include <QStringList>

HistoryIntersectionFilterPrivate::HistoryIntersectionFilterPrivate()
{
}

HistoryIntersectionFilterPrivate::~HistoryIntersectionFilterPrivate()
{
}

HistoryIntersectionFilter::HistoryIntersectionFilter()
    : HistoryFilter(*new HistoryIntersectionFilterPrivate())
{
}

HistoryIntersectionFilter::~HistoryIntersectionFilter()
{
}

void HistoryIntersectionFilter::setFilters(const QList<HistoryFilterPtr> &filters)
{
    Q_D(HistoryIntersectionFilter);
    d->filters = filters;
}

void HistoryIntersectionFilter::prepend(const HistoryFilterPtr &filter)
{
    Q_D(HistoryIntersectionFilter);
    d->filters.prepend(filter);
}

void HistoryIntersectionFilter::append(const HistoryFilterPtr &filter)
{
    Q_D(HistoryIntersectionFilter);
    d->filters.append(filter);
}

void HistoryIntersectionFilter::clear()
{
    Q_D(HistoryIntersectionFilter);
    d->filters.clear();
}

QList<HistoryFilterPtr> HistoryIntersectionFilter::filters() const
{
    Q_D(const HistoryIntersectionFilter);
    return d->filters;
}

QString HistoryIntersectionFilter::toString() const
{
    Q_D(const HistoryIntersectionFilter);

    if (d->filters.isEmpty()) {
        return QString::null;
    } else if (d->filters.count() == 1) {
        return d->filters.first()->toString();
    }

    QStringList output;
    // wrap each filter string around parenthesis
    Q_FOREACH(const HistoryFilterPtr &filter, d->filters) {
        output << QString("(%1)").arg(filter->toString());
    }

    return output.join(" AND ");
}
