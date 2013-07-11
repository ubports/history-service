#include "sort.h"
#include "sort_p.h"

namespace History
{

// ------------- SortPrivate ------------------------------------------------

SortPrivate::SortPrivate(const QString &theSortField,
                                       Qt::SortOrder theSortOrder,
                                       Qt::CaseSensitivity theSortCase)
    : sortField(theSortField), sortOrder(theSortOrder), sortCase(theSortCase)
{
}


SortPrivate::~SortPrivate()
{
}

// ------------- Sort -------------------------------------------------------

Sort::Sort(const QString &sortField,
                         Qt::SortOrder sortOrder,
                         Qt::CaseSensitivity sortCase)
    : d_ptr(new SortPrivate(sortField, sortOrder, sortCase))
{
}

Sort::Sort(const Sort &other)
    : d_ptr(other.d_ptr)
{
}

Sort::~Sort()
{
}

Sort &Sort::operator=(const Sort &other)
{
    if (&other == this) {
        return *this;
    }

    d_ptr = other.d_ptr;
    return *this;
}

QString Sort::sortField()
{
    Q_D(const Sort);
    return d->sortField;
}

Qt::SortOrder Sort::sortOrder()
{
    Q_D(const Sort);
    return d->sortOrder;
}

Qt::CaseSensitivity Sort::sortCase()
{
    Q_D(const Sort);
    return d->sortCase;
}

SortPrivate *Sort::d_func()
{
    return d_ptr.data();
}

}
