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

#include "sort.h"
#include "sort_p.h"

namespace History
{

// ------------- SortPrivate ------------------------------------------------

SortPrivate::SortPrivate(const QString &theSortField,
                                       Qt::SortOrder theSortOrder,
                                       Qt::CaseSensitivity theSortCase)
    : sortField(theSortField), sortOrder(theSortOrder), sortCase(theSortCase)
{
}


SortPrivate::~SortPrivate()
{
}

// ------------- Sort -------------------------------------------------------

Sort::Sort(const QString &sortField,
                         Qt::SortOrder sortOrder,
                         Qt::CaseSensitivity sortCase)
    : d_ptr(new SortPrivate(sortField, sortOrder, sortCase))
{
}

Sort::Sort(const Sort &other)
    : d_ptr(other.d_ptr)
{
}

Sort::~Sort()
{
}

Sort &Sort::operator=(const Sort &other)
{
    if (&other == this) {
        return *this;
    }

    d_ptr = other.d_ptr;
    return *this;
}

QString Sort::sortField()
{
    Q_D(const Sort);
    return d->sortField;
}

Qt::SortOrder Sort::sortOrder()
{
    Q_D(const Sort);
    return d->sortOrder;
}

Qt::CaseSensitivity Sort::sortCase()
{
    Q_D(const Sort);
    return d->sortCase;
}

SortPrivate *Sort::d_func()
{
    return d_ptr.data();
}

}
