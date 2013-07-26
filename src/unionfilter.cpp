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

namespace History
{

UnionFilterPrivate::UnionFilterPrivate()
{
}

UnionFilterPrivate::~UnionFilterPrivate()
{
}

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

void UnionFilter::prepend(const FilterPtr &filter)
{
    Q_D(UnionFilter);
    d->filters.prepend(filter);
}

void UnionFilter::append(const FilterPtr &filter)
{
    Q_D(UnionFilter);
    d->filters.append(filter);
}

void UnionFilter::clear()
{
    Q_D(UnionFilter);
    d->filters.clear();
}

bool UnionFilter::match(const QVariantMap properties) const
{
    Q_D(const UnionFilter);

    // if the filter list is empty, assume it matches
    if (d->filters.isEmpty()) {
        return true;
    }

    // return true if any of the filters match
    Q_FOREACH(const History::FilterPtr &filter, d->filters) {
        if (filter->match(properties)) {
            return true;
        }
    }

    // if we reach this point it means none of the filters matched the properties
    return false;
}

Filters UnionFilter::filters() const
{
    Q_D(const UnionFilter);
    return d->filters;
}

QString UnionFilter::toString(const QString &propertyPrefix) const
{
    Q_D(const UnionFilter);

    if (d->filters.isEmpty()) {
        return QString::null;
    } else if (d->filters.count() == 1) {
        return d->filters.first()->toString();
    }

    QStringList output;
    // wrap each filter string around parenthesis
    Q_FOREACH(const FilterPtr &filter, d->filters) {
        QString value = filter->toString(propertyPrefix);
        if (!value.isEmpty()) {
            output << QString("(%1)").arg(value);
        }
    }

    return output.join(" OR ");
}

}
