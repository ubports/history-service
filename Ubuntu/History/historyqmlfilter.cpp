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

#include "historyqmlfilter.h"
#include "filter.h"

HistoryQmlFilter::HistoryQmlFilter(QObject *parent) :
    QObject(parent), mFilter(new History::Filter())
{
    connect(this,
            SIGNAL(filterPropertyChanged()),
            SIGNAL(filterChanged()));
    connect(this,
            SIGNAL(filterValueChanged()),
            SIGNAL(filterChanged()));
    connect(this,
            SIGNAL(matchFlagsChanged()),
            SIGNAL(filterChanged()));
}

QString HistoryQmlFilter::filterProperty() const
{
    return mFilter->filterProperty();
}

void HistoryQmlFilter::setFilterProperty(const QString &value)
{
    mFilter->setFilterProperty(value);
    Q_EMIT filterPropertyChanged();
}

QVariant HistoryQmlFilter::filterValue() const
{
    return mFilter->filterProperty();
}

void HistoryQmlFilter::setFilterValue(const QVariant &value)
{
    mFilter->setFilterValue(value);
    Q_EMIT filterValueChanged();
}

int HistoryQmlFilter::matchFlags() const
{
    return mFilter->matchFlags();
}

void HistoryQmlFilter::setMatchFlags(int flags)
{
    mFilter->setMatchFlags((History::MatchFlags)flags);
    Q_EMIT matchFlagsChanged();
}


History::FilterPtr HistoryQmlFilter::filter() const
{
    return mFilter;
}


HistoryQmlCompoundFilter::HistoryQmlCompoundFilter(QObject *parent)
    : HistoryQmlFilter(parent)
{
}

HistoryQmlCompoundFilter::~HistoryQmlCompoundFilter()
{
}

QQmlListProperty<HistoryQmlFilter> HistoryQmlCompoundFilter::filters()
{
    return QQmlListProperty<HistoryQmlFilter>(this,
                                              0, // opaque data
                                              filtersAppend,
                                              filtersCount,
                                              filtersAt,
                                              filtersClear);
}

void HistoryQmlCompoundFilter::filtersAppend(QQmlListProperty<HistoryQmlFilter> *prop, HistoryQmlFilter *filter)
{
    HistoryQmlCompoundFilter* compoundFilter = static_cast<HistoryQmlCompoundFilter*>(prop->object);
    compoundFilter->mFilters.append(filter);
    QObject::connect(filter, SIGNAL(filterChanged()), compoundFilter, SIGNAL(filterChanged()), Qt::UniqueConnection);
    Q_EMIT compoundFilter->filterChanged();
}

int HistoryQmlCompoundFilter::filtersCount(QQmlListProperty<HistoryQmlFilter> *prop)
{
    HistoryQmlCompoundFilter *compoundFilter = static_cast<HistoryQmlCompoundFilter*>(prop->object);
    return compoundFilter->mFilters.count();
}

HistoryQmlFilter *HistoryQmlCompoundFilter::filtersAt(QQmlListProperty<HistoryQmlFilter> *prop, int index)
{
    HistoryQmlCompoundFilter* compoundFilter = static_cast<HistoryQmlCompoundFilter*>(prop->object);
    return compoundFilter->mFilters[index];
}

void HistoryQmlCompoundFilter::filtersClear(QQmlListProperty<HistoryQmlFilter> *prop)
{
    HistoryQmlCompoundFilter* compoundFilter = static_cast<HistoryQmlCompoundFilter*>(prop->object);
    if (!compoundFilter->mFilters.isEmpty()) {
        Q_FOREACH(HistoryQmlFilter *filter, compoundFilter->mFilters) {
            filter->disconnect(compoundFilter);
        }

        compoundFilter->mFilters.clear();
    }
}
