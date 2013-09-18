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

#include "historyqmlsort.h"
#include "sort.h"

HistoryQmlSort::HistoryQmlSort(QObject *parent) :
    QObject(parent)
{
    connect(this,
            SIGNAL(sortFieldChanged()),
            SIGNAL(sortChanged()));
    connect(this,
            SIGNAL(sortOrderChanged()),
            SIGNAL(sortChanged()));
    connect(this,
            SIGNAL(caseSensitivityChanged()),
            SIGNAL(sortChanged()));
}

QString HistoryQmlSort::sortField() const
{
    return mSort.sortField();
}

void HistoryQmlSort::setSortField(const QString &value)
{
    mSort.setSortField(value);
    Q_EMIT sortFieldChanged();
}

HistoryQmlSort::SortOrder HistoryQmlSort::sortOrder() const
{
    return (SortOrder) mSort.sortOrder();
}

void HistoryQmlSort::setSortOrder(HistoryQmlSort::SortOrder order)
{
    mSort.setSortOrder((Qt::SortOrder) order);
    Q_EMIT sortOrderChanged();
}

HistoryQmlSort::CaseSensitivity HistoryQmlSort::caseSensitivity() const
{
    return (CaseSensitivity) mSort.caseSensitivity();
}

void HistoryQmlSort::setCaseSensitivity(HistoryQmlSort::CaseSensitivity value)
{
    mSort.setCaseSensitivity((Qt::CaseSensitivity) value);
    Q_EMIT caseSensitivityChanged();
}

History::Sort HistoryQmlSort::sort() const
{
    return mSort;
}
