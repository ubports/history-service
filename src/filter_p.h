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

#ifndef HISTORY_FILTER_P_H
#define HISTORY_FILTER_P_H

#include <QSharedData>
#include <QString>
#include <QVariant>
#include "types.h"

#define HISTORY_FILTER_DECLARE_CLONE(Class) \
    virtual FilterPrivate *clone() { return new Class##Private(*this); }

#define HISTORY_FILTER_DEFINE_COPY(Class, Type) \
    Class::Class(const Filter &other) { \
        if (other.type() == Type) { d_ptr = QSharedPointer<Class##Private>(reinterpret_cast<Class##Private*>(FilterPrivate::getD(other)->clone())); } \
        else { d_ptr = QSharedPointer<Class##Private>(new Class##Private()); } \
    } \
    Class& Class::operator=(const Filter &other) { \
        if (other.type() == Type) { d_ptr = QSharedPointer<Class##Private>(reinterpret_cast<Class##Private*>(FilterPrivate::getD(other)->clone())); } \
        return  *this; \
    }

namespace History
{

class FilterPrivate
{

public:
    FilterPrivate();
    FilterPrivate(const QString &theFilterProperty,
                         const QVariant &theFilterValue,
                         MatchFlags theMatchFlags);
    virtual ~FilterPrivate();


    QString filterProperty;
    QVariant filterValue;
    MatchFlags matchFlags;

    static const QSharedPointer<FilterPrivate>& getD(const Filter& other) { return other.d_ptr; }

    virtual QString toString(const QString &propertyPrefix = QString::null) const;
    virtual bool match(const QVariantMap properties) const;
    virtual FilterType type() const { return History::FilterTypeStandard; }
    virtual bool isValid() const { return (!filterProperty.isNull()) && (!filterValue.isNull()); }
    virtual QVariantMap properties() const;

    HISTORY_FILTER_DECLARE_CLONE(Filter)
};

}

#endif // HISTORY_FILTER_P_H
