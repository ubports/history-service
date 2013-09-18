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

#ifndef HISTORY_INTERSECTIONFILTER_P_H
#define HISTORY_INTERSECTIONFILTER_P_H

#include "filter_p.h"

namespace History
{

class IntersectionFilter;

class IntersectionFilterPrivate : public FilterPrivate
{

public:
    IntersectionFilterPrivate();
    ~IntersectionFilterPrivate();
    virtual FilterType type() const { return FilterTypeIntersection; }
    QString toString(const QString &propertyPrefix = QString::null) const;
    bool match(const QVariantMap properties) const;
    bool isValid() const;

    Filters filters;

    HISTORY_FILTER_DECLARE_CLONE(IntersectionFilter)
};

}

#endif // HISTORY_INTERSECTIONFILTER_P_H
