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

#ifndef HISTORYQMLFILTER_H
#define HISTORYQMLFILTER_H

#include <qqml.h>
#include <QObject>
#include "types.h"
#include "filter.h"

class HistoryQmlFilter : public QObject
{
    Q_OBJECT
    Q_ENUMS(MatchFlag)
    Q_PROPERTY(QString filterProperty READ filterProperty WRITE setFilterProperty NOTIFY filterPropertyChanged)
    Q_PROPERTY(QVariant filterValue READ filterValue WRITE setFilterValue NOTIFY filterValueChanged)
    Q_PROPERTY(int matchFlags READ matchFlags WRITE setMatchFlags NOTIFY matchFlagsChanged)
public:
    enum MatchFlag {
        MatchCaseSensitive = History::MatchCaseSensitive,
        MatchCaseInsensitive = History::MatchCaseInsensitive,
        MatchContains = History::MatchContains,
        MatchPhoneNumber = History::MatchPhoneNumber,
        MatchNotEquals = History::MatchNotEquals,
        MatchLess = History::MatchLess,
        MatchGreater = History::MatchGreater,
        MatchLessOrEquals = History::MatchLessOrEquals,
        MatchGreaterOrEquals = History::MatchGreaterOrEquals
    };

    explicit HistoryQmlFilter(QObject *parent = 0);

    QString filterProperty() const;
    void setFilterProperty(const QString &value);

    QVariant filterValue() const;
    void setFilterValue(const QVariant &value);

    int matchFlags() const;
    void setMatchFlags(int flags);

    virtual History::Filter filter() const;

Q_SIGNALS:
    void filterPropertyChanged();
    void filterValueChanged();
    void matchFlagsChanged();
    void filterChanged();
    
protected:
    History::Filter mFilter;
};


// compound filter
class HistoryQmlCompoundFilter : public HistoryQmlFilter
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<HistoryQmlFilter> filters READ filters NOTIFY filtersChanged)
    Q_CLASSINFO("DefaultProperty", "filters")

public:
    explicit HistoryQmlCompoundFilter(QObject* parent = 0);
    virtual ~HistoryQmlCompoundFilter();
    QQmlListProperty<HistoryQmlFilter> filters();

    static void filtersAppend(QQmlListProperty<HistoryQmlFilter>* prop, HistoryQmlFilter* filter);
    static int filtersCount(QQmlListProperty<HistoryQmlFilter>* prop);
    static HistoryQmlFilter* filtersAt(QQmlListProperty<HistoryQmlFilter>* prop, int index);
    static void filtersClear(QQmlListProperty<HistoryQmlFilter>* prop);

Q_SIGNALS:
    void filtersChanged();

protected:
    QList<HistoryQmlFilter*> mFilters;
};

#endif // HISTORYQMLFILTER_H
