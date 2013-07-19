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

namespace History
{

IntersectionFilterPrivate::IntersectionFilterPrivate()
{
}

IntersectionFilterPrivate::~IntersectionFilterPrivate()
{
}

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

void IntersectionFilter::prepend(const FilterPtr &filter)
{
    Q_D(IntersectionFilter);
    d->filters.prepend(filter);
}

void IntersectionFilter::append(const FilterPtr &filter)
{
    Q_D(IntersectionFilter);
    d->filters.append(filter);
}

void IntersectionFilter::clear()
{
    Q_D(IntersectionFilter);
    d->filters.clear();
}

bool IntersectionFilter::match(const QVariantMap properties) const
{
    Q_D(const IntersectionFilter);

    // return true only if all filters match
    Q_FOREACH(const History::FilterPtr &filter, d->filters) {
        if (!filter->match(properties)) {
            return false;
        }
    }

    return true;
}

Filters IntersectionFilter::filters() const
{
    Q_D(const IntersectionFilter);
    return d->filters;
}

QString IntersectionFilter::toString() const
{
    Q_D(const IntersectionFilter);

    if (d->filters.isEmpty()) {
        return QString::null;
    } else if (d->filters.count() == 1) {
        return d->filters.first()->toString();
    }

    QStringList output;
    // wrap each filter string around parenthesis
    Q_FOREACH(const FilterPtr &filter, d->filters) {
        output << QString("(%1)").arg(filter->toString());
    }

    return output.join(" AND ");
}

}
