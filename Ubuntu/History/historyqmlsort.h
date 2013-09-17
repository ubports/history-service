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

#ifndef HISTORYQMLSORT_H
#define HISTORYQMLSORT_H

#include <QObject>
#include "types.h"
#include "sort.h"

class HistoryQmlSort : public QObject
{
    Q_OBJECT
    Q_ENUMS(SortOrder)
    Q_ENUMS(CaseSensitivity)
    Q_PROPERTY(QString sortField READ sortField WRITE setSortField NOTIFY sortFieldChanged)
    Q_PROPERTY(SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(CaseSensitivity caseSensitivity READ CaseSensitivity WRITE setCaseSensitivity NOTIFY caseSensitivityChanged)

public:
    enum SortOrder {
        AscendingOrder = Qt::AscendingOrder,
        DescendingOrder = Qt::DescendingOrder
    };

    enum CaseSensitivity {
        CaseInsensitive = Qt::CaseInsensitive,
        CaseSensitive = Qt::CaseSensitive
    };

    explicit HistoryQmlSort(QObject *parent = 0);

    QString sortField() const;
    void setSortField(const QString &value);

    SortOrder sortOrder() const;
    void setSortOrder(SortOrder order);

    CaseSensitivity caseSensitivity() const;
    void setCaseSensitivity(CaseSensitivity value);

    History::Sort sort() const;

Q_SIGNALS:
    void sortChanged();
    void sortFieldChanged();
    void sortOrderChanged();
    void caseSensitivityChanged();

private:
    History::Sort mSort;
};

#endif // HISTORYQMLSORT_H
