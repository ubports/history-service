#include "historyunionfilter.h"
#include "historyunionfilter_p.h"
#include <QStringList>

HistoryUnionFilterPrivate::HistoryUnionFilterPrivate()
{
}

HistoryUnionFilterPrivate::~HistoryUnionFilterPrivate()
{
}

HistoryUnionFilter::HistoryUnionFilter()
    : HistoryFilter(*new HistoryUnionFilterPrivate())
{
}

HistoryUnionFilter::HistoryUnionFilter(const HistoryUnionFilter &other)
    : HistoryFilter(other)
{
}

HistoryUnionFilter::~HistoryUnionFilter()
{
}

void HistoryUnionFilter::setFilters(const QList<HistoryFilter> &filters)
{
    Q_D(HistoryUnionFilter);
    d->filters = filters;
}

void HistoryUnionFilter::prepend(const HistoryFilter &filter)
{
    Q_D(HistoryUnionFilter);
    d->filters.prepend(filter);
}

void HistoryUnionFilter::append(const HistoryFilter &filter)
{
    Q_D(HistoryUnionFilter);
    d->filters.append(filter);
}

QList<HistoryFilter> HistoryUnionFilter::filters() const
{
    Q_D(const HistoryUnionFilter);
    return d->filters;
}

QString HistoryUnionFilter::toString() const
{
    Q_D(const HistoryUnionFilter);

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

    return output.join(" OR ");
}

HistoryUnionFilterPrivate *HistoryUnionFilter::d_func()
{
    return reinterpret_cast<HistoryUnionFilterPrivate *>(d_ptr.data());
}

const HistoryUnionFilterPrivate *HistoryUnionFilter::d_func() const
{
    return reinterpret_cast<const HistoryUnionFilterPrivate *>(d_ptr.constData());
}


