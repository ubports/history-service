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
#include <typeinfo>
#include <QDebug>

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

QString FilterPrivate::toString(const QString &propertyPrefix) const
{
    if (filterProperty.isEmpty() || filterValue.isNull()) {
        return QString::null;
    }

    QString value;

    switch (filterValue.type()) {
    case QVariant::String:
        // FIXME: need to escape strings
        // wrap strings
        value = QString("\"%1\"").arg(filterValue.toString());
        break;
    default:
        value = filterValue.toString();
    }

    QString propertyName = propertyPrefix.isNull() ? filterProperty : QString("%1.%2").arg(propertyPrefix, filterProperty);
    // FIXME2: need to check for the match flags
    return QString("%1=%2").arg(propertyName, value);
}

bool FilterPrivate::match(const QVariantMap properties) const
{
    // assume empty filters match anything
    if (filterProperty.isEmpty() || !filterValue.isValid() || !properties.contains(filterProperty)) {
        return true;
    }

    // FIXME: use the MatchFlags
    return properties[filterProperty] == filterValue;
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

Filter::Filter(const Filter &other)
    : d_ptr(other.d_ptr->clone())
{
}

Filter::~Filter()
{
}

Filter &Filter::operator=(const Filter &other)
{
    if (&other == this) {
        return *this;
    }

    d_ptr = QSharedPointer<FilterPrivate>(other.d_ptr->clone());
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

QString Filter::toString(const QString &propertyPrefix) const
{
    Q_D(const Filter);
    return d->toString(propertyPrefix);
}

bool Filter::match(const QVariantMap properties) const
{
    Q_D(const Filter);
    return d->match(properties);
}

FilterType Filter::type() const
{
    Q_D(const Filter);
    return d->type();
}

bool Filter::operator==(const Filter &other) const
{
    // FIXME: implement in a more performant way
    return toString() == other.toString();
}

bool Filter::isValid() const
{
    Q_D(const Filter);
    return d->isValid();
}

}
