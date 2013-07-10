#include "historyqmlfilter.h"

HistoryQmlFilter::HistoryQmlFilter(QObject *parent) :
    QObject(parent), mFilter(new HistoryFilter())
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
    mFilter->setMatchFlags((HistoryFilter::MatchFlags)flags);
    Q_EMIT matchFlagsChanged();
}


HistoryFilterPtr HistoryQmlFilter::filter() const
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
