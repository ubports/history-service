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

QObject* SortProxyModel::model() const
{

    return sourceModel();
}

void SortProxyModel::setModel(QObject* value)
{
    QAbstractItemModel *model = qobject_cast<QAbstractItemModel*>(value);
    connect(model,SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(updateSorting()));
    setSourceModel(model);
    Q_EMIT modelChanged();
}

void SortProxyModel::updateSorting()
{
    sort(0, mAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

QVariant SortProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    return QSortFilterProxyModel::data(index, role);
}
