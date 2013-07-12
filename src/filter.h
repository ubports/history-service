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

#ifndef HISTORY_FILTER_H
#define HISTORY_FILTER_H

#include <QFlags>
#include <QScopedPointer>
#include <QVariant>
#include "types.h"

namespace History
{

class FilterPrivate;

// simple filter
class Filter
{
    Q_DECLARE_PRIVATE(Filter)
public:
    Filter(const QString &filterProperty = QString::null,
                  const QVariant &filterValue = QVariant(),
                  MatchFlags matchFlags = MatchCaseSensitive);
    virtual ~Filter();

    QString filterProperty() const;
    void setFilterProperty(const QString &value);

    QVariant filterValue() const;
    void setFilterValue(const QVariant &value);

    MatchFlags matchFlags() const;
    void setMatchFlags(const MatchFlags &flags);
    virtual QString toString() const;

protected:
    Filter(FilterPrivate &p);
    QScopedPointer<FilterPrivate> d_ptr;
};

}

#endif
