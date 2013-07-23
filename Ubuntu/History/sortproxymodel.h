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

#ifndef SORTPROXYMODEL_H
#define SORTPROXYMODEL_H

#include <QSortFilterProxyModel>

class SortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool ascending
               READ ascending
               WRITE setAscending
               NOTIFY ascendingChanged)
    Q_PROPERTY(QObject* model
               READ model
               WRITE setModel
               NOTIFY modelChanged)
public:
    explicit SortProxyModel(QObject *parent = 0);

    bool ascending() const;
    void setAscending(bool value);

    QObject* model() const;
    void setModel(QObject* value);


    virtual QVariant data(const QModelIndex &index, int role) const;

private Q_SLOTS:
    void updateSorting();

Q_SIGNALS:
    void ascendingChanged();
    void modelChanged();

private:
    bool mAscending;
};

#endif // SORTPROXYMODEL_H
