/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
