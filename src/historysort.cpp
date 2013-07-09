#include "historysort.h"
#include "historysort_p.h"

// ------------- HistorySortPrivate ------------------------------------------------

HistorySortPrivate::HistorySortPrivate(const QString &theSortField,
                                       Qt::SortOrder theSortOrder,
                                       Qt::CaseSensitivity theSortCase)
    : sortField(theSortField), sortOrder(theSortOrder), sortCase(theSortCase)
{
}


HistorySortPrivate::~HistorySortPrivate()
{
}

// ------------- HistorySort -------------------------------------------------------

HistorySort::HistorySort(const QString &sortField,
                         Qt::SortOrder sortOrder,
                         Qt::CaseSensitivity sortCase)
    : d_ptr(new HistorySortPrivate(sortField, sortOrder, sortCase))
{
}

HistorySort::HistorySort(const HistorySort &other)
    : d_ptr(other.d_ptr)
{
}

HistorySort::~HistorySort()
{
}

HistorySort &HistorySort::operator=(const HistorySort &other)
{
    if (&other == this) {
        return *this;
    }

    d_ptr = other.d_ptr;
    return *this;
}

QString HistorySort::sortField()
{
    Q_D(const HistorySort);
    return d->sortField;
}

Qt::SortOrder HistorySort::sortOrder()
{
    Q_D(const HistorySort);
    return d->sortOrder;
}

Qt::CaseSensitivity HistorySort::sortCase()
{
    Q_D(const HistorySort);
    return d->sortCase;
}

HistorySortPrivate *HistorySort::d_func()
{
    return d_ptr.data();
}
