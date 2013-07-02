#include "historyfilter.h"
#include "historyfilter_p.h"

// ------------- HistoryFilterPrivate ------------------------------------------------

HistoryFilterPrivate::HistoryFilterPrivate()
{
}

HistoryFilterPrivate::HistoryFilterPrivate(const QString &theFilterProperty,
                                           const QVariant &theFilterValue,
                                           HistoryFilter::MatchFlags theMatchFlags)
    : filterProperty(theFilterProperty), filterValue(theFilterValue), matchFlags(theMatchFlags)
{
}

HistoryFilterPrivate::~HistoryFilterPrivate()
{
}

// ------------- HistoryFilter -------------------------------------------------------

HistoryFilter::HistoryFilter(HistoryFilterPrivate &p)
    : d_ptr(&p)
{
}

HistoryFilterPrivate *HistoryFilter::d_func()
{
    return d_ptr.data();
}

HistoryFilter::HistoryFilter(const QString &filterProperty,
                             const QVariant &filterValue,
                             HistoryFilter::MatchFlags matchFlags)
    : d_ptr(new HistoryFilterPrivate(filterProperty, filterValue, matchFlags))
{
}

HistoryFilter::HistoryFilter(const HistoryFilter &other)
    : d_ptr(other.d_ptr)
{
}

HistoryFilter::~HistoryFilter()
{
}

QString HistoryFilter::filterProperty() const
{
    Q_D(const HistoryFilter);
    return d->filterProperty;
}

void HistoryFilter::setFilterProperty(const QString &value)
{
    Q_D(HistoryFilter);
    d->filterProperty = value;
}

QVariant HistoryFilter::filterValue() const
{
    Q_D(const HistoryFilter);
    return d->filterValue;
}

void HistoryFilter::setFilterValue(const QVariant &value)
{
    Q_D(HistoryFilter);
    d->filterValue = value;
}

HistoryFilter::MatchFlags HistoryFilter::matchFlags() const
{
    Q_D(const HistoryFilter);
    return d->matchFlags;
}

void HistoryFilter::setMatchFlags(const HistoryFilter::MatchFlags &flags)
{
    Q_D(HistoryFilter);
    d->matchFlags = flags;
}

QString HistoryFilter::toString() const
{
    Q_D(const HistoryFilter);

    if (d->filterProperty.isEmpty()) {
        return QString::null;
    }

    QString value;

    switch (d->filterValue.type()) {
    case QVariant::String:
        // FIXME: need to escape strings
        // wrap strings
        value = QString("\"%1\"").arg(d->filterValue.toString());
        break;
    default:
        value = d->filterValue.toString();
    }

    // FIXME2: need to check for the match flags
    return QString("%1=%2").arg(filterProperty(), value);
}
