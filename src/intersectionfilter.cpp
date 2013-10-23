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

#include "intersectionfilter.h"
#include "intersectionfilter_p.h"
#include <QStringList>
#include <QDebug>

namespace History
{

IntersectionFilterPrivate::IntersectionFilterPrivate()
{
}

IntersectionFilterPrivate::~IntersectionFilterPrivate()
{
}

bool IntersectionFilterPrivate::match(const QVariantMap properties) const
{
    // return true only if all filters match
    Q_FOREACH(const Filter &filter, filters) {
        if (!filter.match(properties)) {
            return false;
        }
    }

    return true;
}

bool IntersectionFilterPrivate::isValid() const
{
    // FIXME: maybe we should check if at least one of the inner filters are valid?
    return !filters.isEmpty();
}

QVariantMap IntersectionFilterPrivate::properties() const
{
    QVariantMap map;
    if (!isValid()) {
        return map;
    }

    QVariantList filterList;
    Q_FOREACH(const Filter &filter, filters) {
        filterList << filter.properties();
    }
    map[FieldFilters] = filterList;
    map[FieldFilterType] = (int) History::FilterTypeIntersection;
    return map;
}

QString IntersectionFilterPrivate::toString(const QString &propertyPrefix) const
{
    if (filters.isEmpty()) {
        return QString::null;
    } else if (filters.count() == 1) {
        return filters.first().toString();
    }

    QStringList output;
    // wrap each filter string around parenthesis
    Q_FOREACH(const Filter &filter, filters) {
        QString value = filter.toString(propertyPrefix);
        if (!value.isEmpty()) {
            output << QString("(%1)").arg(value);
        }
    }

    return output.join(" AND ");
}


HISTORY_FILTER_DEFINE_COPY(IntersectionFilter, FilterTypeIntersection)

IntersectionFilter::IntersectionFilter()
    : Filter(*new IntersectionFilterPrivate())
{
}

IntersectionFilter::~IntersectionFilter()
{
}

void IntersectionFilter::setFilters(const Filters &filters)
{
    Q_D(IntersectionFilter);
    d->filters = filters;
}

void IntersectionFilter::prepend(const Filter &filter)
{
    Q_D(IntersectionFilter);
    d->filters.prepend(filter);
}

void IntersectionFilter::append(const Filter &filter)
{
    Q_D(IntersectionFilter);
    d->filters.append(filter);
}

void IntersectionFilter::clear()
{
    Q_D(IntersectionFilter);
    d->filters.clear();
}

Filters IntersectionFilter::filters() const
{
    Q_D(const IntersectionFilter);
    return d->filters;
}

Filter IntersectionFilter::fromProperties(const QVariantMap &properties)
{
    IntersectionFilter filter;
    if (properties.isEmpty()) {
        return filter;
    }

    QVariantList filters = properties[FieldFilters].toList();
    Q_FOREACH(const QVariant &props, filters) {
        Filter innerFilter = History::Filter::fromProperties(props.toMap());
        if (innerFilter.isValid()) {
            filter.append(innerFilter);
        }
    }

    return filter;
}

}
