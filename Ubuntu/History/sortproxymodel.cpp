/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "sortproxymodel.h"
#include <QDebug>

SortProxyModel::SortProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent), mAscending(true)
{
    setDynamicSortFilter(true);
    updateSorting();

    connect(this, SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(countChanged()));
    connect(this, SIGNAL(modelReset()), SIGNAL(countChanged()));
}

bool SortProxyModel::ascending() const
{
    return mAscending;
}

void SortProxyModel::setAscending(bool value)
{
    if (mAscending != value) {
        mAscending = value;
        updateSorting();
        Q_EMIT ascendingChanged();
    }
}

QVariant SortProxyModel::get(int row) const
{
    QVariantMap data;
    QModelIndex sourceIndex = mapToSource(index(row, 0));
    if (sourceIndex.isValid()) {
        QAbstractItemModel *source = sourceModel();
        QHash<int, QByteArray> roles = source->roleNames();
        Q_FOREACH(int role, roles.keys()) {
            data.insert(roles[role], source->data(sourceIndex, role));
        }
    }

    return data;
}

void SortProxyModel::updateSorting()
{
    sort(0, mAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}
