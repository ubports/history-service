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
#include "intersectionfilter.h"
#include "unionfilter.h"
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
    // FIXME: remove the toString() functionality or replace it by a better implementation
    if (filterProperty.isEmpty() || filterValue.isNull()) {
        return QString();
    }

    QString value;

    switch (filterValue.type()) {
    case QVariant::String:
        // FIXME: need to escape strings
        // wrap strings
        value = QString("\"%1\"").arg(filterValue.toString());
        break;
    case QVariant::Bool:
        value = filterValue.toBool() ? "1" : "0";
        break;
    case QVariant::Int:
        value = QString::number(filterValue.toInt());
        break;
    case QVariant::Double:
        value = QString::number(filterValue.toDouble());
        break;
    default:
        value = filterValue.toString();
    }

    QString propertyName = propertyPrefix.isNull() ? filterProperty : QString("%1.%2").arg(propertyPrefix, filterProperty);
    // FIXME2: need to check for the match flags
    QString condition;
    switch (matchFlags) {
    case History::MatchNotEquals:
        condition = QString("%1!=%2");
        break;
    case History::MatchLess:
        condition = QString("%1<%2");
        break;
    case History::MatchGreater:
        condition = QString("%1>%2");
        break;
    case History::MatchLessOrEquals:
        condition = QString("%1<=%2");
        break;
    case History::MatchGreaterOrEquals:
        condition = QString("%1>=%2");
        break;
    default:
        condition = QString("%1=%2");
    }

    return QString(condition).arg(propertyName, value);
}

bool FilterPrivate::match(const QVariantMap properties) const
{
    // assume empty filters match anything
    if (filterProperty.isEmpty() || !filterValue.isValid() || !properties.contains(filterProperty)) {
        return true;
    }

    switch (matchFlags) {
    case History::MatchNotEquals:
        return properties[filterProperty] != filterValue;
        break;
    case History::MatchLess:
        return properties[filterProperty] < filterValue;
        break;
    case History::MatchGreater:
        return properties[filterProperty] > filterValue;
        break;
    case History::MatchLessOrEquals:
        return properties[filterProperty] <= filterValue;
        break;
    case History::MatchGreaterOrEquals:
        return properties[filterProperty] >= filterValue;
        break;
    default:
        return properties[filterProperty] == filterValue;
    }
}

QVariantMap FilterPrivate::properties() const
{
    QVariantMap map;
    if (!isValid()) {
        return map;
    }
    map[FieldFilterType] = (int)FilterTypeStandard;
    map[FieldFilterProperty] = filterProperty;
    map[FieldFilterValue] = filterValue;
    map[FieldMatchFlags] = (int)matchFlags;
    return map;
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
    return *this;
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

QVariantMap Filter::properties() const
{
    Q_D(const Filter);
    return d->properties();
}

Filter Filter::fromProperties(const QVariantMap &properties)
{
    Filter filter;
    if (properties.isEmpty()) {
        return filter;
    }

    switch ((FilterType)properties[FieldFilterType].toInt()) {
    case FilterTypeStandard:
        filter = Filter(properties[FieldFilterProperty].toString(), properties[FieldFilterValue], (MatchFlags)properties[FieldMatchFlags].toInt());
        break;
    case FilterTypeIntersection:
        filter = IntersectionFilter::fromProperties(properties);
        break;
    case FilterTypeUnion:
        filter = UnionFilter::fromProperties(properties);
        break;
    }
    return filter;
}

}
