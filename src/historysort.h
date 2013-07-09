#ifndef HISTORYSORT_H
#define HISTORYSORT_H

#include <QSharedDataPointer>
#include <QString>
#include <Qt>

class HistorySortPrivate;

class HistorySort
{

public:
    HistorySort(const QString &sortField = "timestamp",
                Qt::SortOrder sortOrder = Qt::AscendingOrder,
                Qt::CaseSensitivity sortCase = Qt::CaseInsensitive);
    HistorySort(const HistorySort &other);
    ~HistorySort();

    HistorySort &operator=(const HistorySort &other);

    QString sortField();
    Qt::SortOrder sortOrder();
    Qt::CaseSensitivity sortCase();

protected:
    QSharedDataPointer<HistorySortPrivate> d_ptr;

    // Q_DECLARE_PRIVATE equivalent for shared data pointers
    HistorySortPrivate *d_func();
    inline const HistorySortPrivate *d_func() const
    {
        return d_ptr.constData();
    }
};

#endif
