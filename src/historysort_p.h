#ifndef HISTORYSORT_P_H
#define HISTORYSORT_P_H

#include <QSharedData>
#include <QString>
#include <Types>

class HistorySort;

class HistorySortPrivate : public QSharedData
{
public:
    HistorySortPrivate(const QString &theSortField,
                       Qt::SortOrder theSortOrder,
                       Qt::CaseSensitivity theSortCase);
    virtual ~HistorySortPrivate();

    QString sortField;
    Qt::SortOrder sortOrder;
    Qt::CaseSensitivity sortCase;
};

#endif // HISTORYSORT_P_H
