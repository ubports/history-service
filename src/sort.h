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

#ifndef HISTORY_SORT_H
#define HISTORY_SORT_H

#include <QScopedPointer>
#include <QString>
#include <Qt>
#include <QVariantMap>
#include "types.h"

namespace History
{

class SortPrivate;

class Sort
{
    Q_DECLARE_PRIVATE(Sort)
public:
    Sort(const QString &sortField = "timestamp",
                Qt::SortOrder sortOrder = Qt::AscendingOrder,
                Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive);
    ~Sort();

    QString sortField() const;
    void setSortField(const QString &value);

    Qt::SortOrder sortOrder() const;
    void setSortOrder(Qt::SortOrder value);

    Qt::CaseSensitivity caseSensitivity() const;
    void setCaseSensitivity(Qt::CaseSensitivity value);

    QVariantMap properties() const;
    static SortPtr fromProperties(const QVariantMap &properties);

protected:
    QScopedPointer<SortPrivate> d_ptr;
};

}

#endif // HISTORY_SORT_H
