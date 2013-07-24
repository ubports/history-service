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
                                       Qt::CaseSensitivity theCaseSensitivity)
    : sortField(theSortField), sortOrder(theSortOrder), caseSensitivity(theCaseSensitivity)
{
}


SortPrivate::~SortPrivate()
{
}

// ------------- Sort -------------------------------------------------------

Sort::Sort(const QString &sortField,
                         Qt::SortOrder sortOrder,
                         Qt::CaseSensitivity caseSensitivity)
    : d_ptr(new SortPrivate(sortField, sortOrder, caseSensitivity))
{
}

Sort::~Sort()
{
}

QString Sort::sortField() const
{
    Q_D(const Sort);
    return d->sortField;
}

void Sort::setSortField(const QString &value)
{
    Q_D(Sort);
    d->sortField = value;
}

Qt::SortOrder Sort::sortOrder() const
{
    Q_D(const Sort);
    return d->sortOrder;
}

void Sort::setSortOrder(Qt::SortOrder value)
{
    Q_D(Sort);
    d->sortOrder = value;
}

Qt::CaseSensitivity Sort::caseSensitivity() const
{
    Q_D(const Sort);
    return d->caseSensitivity;
}

void Sort::setCaseSensitivity(Qt::CaseSensitivity value)
{
    Q_D(Sort);
    d->caseSensitivity = value;
}

}
