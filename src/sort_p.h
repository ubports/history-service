#ifndef HISTORY_SORT_P_H
#define HISTORY_SORT_P_H

#include <QSharedData>
#include <QString>
#include "types.h"

namespace History
{

class Sort;

class SortPrivate : public QSharedData
{
public:
    SortPrivate(const QString &theSortField,
                       Qt::SortOrder theSortOrder,
                       Qt::CaseSensitivity theSortCase);
    virtual ~SortPrivate();

    QString sortField;
    Qt::SortOrder sortOrder;
    Qt::CaseSensitivity sortCase;
};

}

#endif // HISTORY_SORT_P_H
