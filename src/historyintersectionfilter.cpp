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

HistoryIntersectionFilter::HistoryIntersectionFilter(const HistoryIntersectionFilter &other)
    : HistoryFilter(other)
{
}

HistoryIntersectionFilter::~HistoryIntersectionFilter()
{
}

void HistoryIntersectionFilter::setFilters(const QList<HistoryFilter> &filters)
{
    Q_D(HistoryIntersectionFilter);
    d->filters = filters;
}

void HistoryIntersectionFilter::prepend(const HistoryFilter &filter)
{
    Q_D(HistoryIntersectionFilter);
    d->filters.prepend(filter);
}

void HistoryIntersectionFilter::append(const HistoryFilter &filter)
{
    Q_D(HistoryIntersectionFilter);
    d->filters.append(filter);
}

QList<HistoryFilter> HistoryIntersectionFilter::filters() const
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
        return d->filters.first().toString();
    }

    QStringList output;
    // wrap each filter string around parenthesis
    Q_FOREACH(const HistoryFilter &filter, d->filters) {
        output << QString("(%1)").arg(filter.toString());
    }

    return output.join(" AND ");
}

HistoryIntersectionFilterPrivate *HistoryIntersectionFilter::d_func()
{
    return reinterpret_cast<HistoryIntersectionFilterPrivate *>(d_ptr.data());
}

const HistoryIntersectionFilterPrivate *HistoryIntersectionFilter::d_func() const
{
    return reinterpret_cast<const HistoryIntersectionFilterPrivate *>(d_ptr.constData());
}


