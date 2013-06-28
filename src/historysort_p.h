#ifndef HISTORYSORT_P_H
#define HISTORYSORT_P_H

#include <QString>
#include <Types>

class HistorySort;

class HistorySortPrivate
{
    Q_DECLARE_PUBLIC(HistorySort)
public:
    HistorySortPrivate(const QString &theSortField,
                       Qt::SortOrder theSortOrder,
                       Qt::CaseSensitivity theSortCase = Qt::CaseInsensitive);
    virtual ~HistorySortPrivate();

    QString sortField;
    Qt::SortOrder sortOrder;
    Qt::CaseSensitivity sortCase;

    HistorySort *q_ptr;
};

#endif // HISTORYSORT_P_H
