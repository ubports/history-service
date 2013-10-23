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

#include "unionfilter.h"
#include "unionfilter_p.h"
#include <QStringList>
#include <QDebug>

namespace History
{

UnionFilterPrivate::UnionFilterPrivate()
{
}

UnionFilterPrivate::~UnionFilterPrivate()
{
}

bool UnionFilterPrivate::match(const QVariantMap properties) const
{
    // if the filter list is empty, assume it matches
    if (filters.isEmpty()) {
        return true;
    }

    // return true if any of the filters match
    Q_FOREACH(const History::Filter &filter, filters) {
        if (filter.match(properties)) {
            return true;
        }
    }

    // if we reach this point it means none of the filters matched the properties
    return false;
}

bool UnionFilterPrivate::isValid() const
{
    // FIXME: maybe we should check if at least one of the inner filters are valid?
    return !filters.isEmpty();
}

QVariantMap UnionFilterPrivate::properties() const
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
    map[FieldFilterType] = (int) History::FilterTypeUnion;
    return map;
}

QString UnionFilterPrivate::toString(const QString &propertyPrefix) const
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

    return output.join(" OR ");
}

HISTORY_FILTER_DEFINE_COPY(UnionFilter, FilterTypeUnion)

UnionFilter::UnionFilter()
    : Filter(*new UnionFilterPrivate())
{
}

UnionFilter::~UnionFilter()
{
}

void UnionFilter::setFilters(const Filters &filters)
{
    Q_D(UnionFilter);
    d->filters = filters;
}

void UnionFilter::prepend(const Filter &filter)
{
    Q_D(UnionFilter);
    d->filters.prepend(filter);
}

void UnionFilter::append(const Filter &filter)
{
    Q_D(UnionFilter);
    d->filters.append(filter);
}

void UnionFilter::clear()
{
    Q_D(UnionFilter);
    d->filters.clear();
}

Filters UnionFilter::filters() const
{
    Q_D(const UnionFilter);
    return d->filters;
}

Filter UnionFilter::fromProperties(const QVariantMap &properties)
{
    UnionFilter filter;
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
