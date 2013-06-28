#ifndef HISTORYSORT_H
#define HISTORYSORT_H

#include <QScopedPointer>
#include <QString>
#include <Qt>

class HistorySortPrivate;

class HistorySort
{
    Q_DECLARE_PRIVATE(HistorySort)

public:
    HistorySort(const QString &sortField = "timestamp",
                Qt::SortOrder sortOrder = Qt::AscendingOrder,
                Qt::CaseSensitivity sortCase = Qt::CaseInsensitive);
    ~HistorySort();

    QString sortField();
    Qt::SortOrder sortOrder();
    Qt::CaseSensitivity sortCase();

protected:
    QScopedPointer<HistorySortPrivate> d_ptr;
};

#endif
