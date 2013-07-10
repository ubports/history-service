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

HistoryUnionFilter::~HistoryUnionFilter()
{
}

void HistoryUnionFilter::setFilters(const QList<HistoryFilterPtr> &filters)
{
    Q_D(HistoryUnionFilter);
    d->filters = filters;
}

void HistoryUnionFilter::prepend(const HistoryFilterPtr &filter)
{
    Q_D(HistoryUnionFilter);
    d->filters.prepend(filter);
}

void HistoryUnionFilter::append(const HistoryFilterPtr &filter)
{
    Q_D(HistoryUnionFilter);
    d->filters.append(filter);
}

void HistoryUnionFilter::clear()
{
    Q_D(HistoryUnionFilter);
    d->filters.clear();
}

QList<HistoryFilterPtr> HistoryUnionFilter::filters() const
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
        return d->filters.first()->toString();
    }

    QStringList output;
    // wrap each filter string around parenthesis
    Q_FOREACH(const HistoryFilterPtr &filter, d->filters) {
        output << QString("(%1)").arg(filter->toString());
    }

    return output.join(" OR ");
}
