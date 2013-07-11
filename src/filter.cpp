#include "filter.h"
#include "filter_p.h"

namespace History
{

// ------------- FilterPrivate ------------------------------------------------

FilterPrivate::FilterPrivate()
{
}

FilterPrivate::FilterPrivate(const QString &theFilterProperty,
                                           const QVariant &theFilterValue,
                                           MatchFlags theMatchFlags)
    : filterProperty(theFilterProperty), filterValue(theFilterValue), matchFlags(theMatchFlags)
{
}

FilterPrivate::~FilterPrivate()
{
}

// ------------- Filter -------------------------------------------------------

Filter::Filter(FilterPrivate &p)
    : d_ptr(&p)
{
}

Filter::Filter(const QString &filterProperty,
                             const QVariant &filterValue,
                             MatchFlags matchFlags)
    : d_ptr(new FilterPrivate(filterProperty, filterValue, matchFlags))
{
}

Filter::~Filter()
{
}

QString Filter::filterProperty() const
{
    Q_D(const Filter);
    return d->filterProperty;
}

void Filter::setFilterProperty(const QString &value)
{
    Q_D(Filter);
    d->filterProperty = value;
}

QVariant Filter::filterValue() const
{
    Q_D(const Filter);
    return d->filterValue;
}

void Filter::setFilterValue(const QVariant &value)
{
    Q_D(Filter);
    d->filterValue = value;
}

MatchFlags Filter::matchFlags() const
{
    Q_D(const Filter);
    return d->matchFlags;
}

void Filter::setMatchFlags(const MatchFlags &flags)
{
    Q_D(Filter);
    d->matchFlags = flags;
}

QString Filter::toString() const
{
    Q_D(const Filter);

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

}
