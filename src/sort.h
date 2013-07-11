#ifndef HISTORY_SORT_H
#define HISTORY_SORT_H

#include <QSharedDataPointer>
#include <QString>
#include <Qt>

namespace History
{

class SortPrivate;

class Sort
{

public:
    Sort(const QString &sortField = "timestamp",
                Qt::SortOrder sortOrder = Qt::AscendingOrder,
                Qt::CaseSensitivity sortCase = Qt::CaseInsensitive);
    Sort(const Sort &other);
    ~Sort();

    Sort &operator=(const Sort &other);

    QString sortField();
    Qt::SortOrder sortOrder();
    Qt::CaseSensitivity sortCase();

protected:
    QSharedDataPointer<SortPrivate> d_ptr;

    // Q_DECLARE_PRIVATE equivalent for shared data pointers
    SortPrivate *d_func();
    inline const SortPrivate *d_func() const
    {
        return d_ptr.constData();
    }
};

}

#endif // HISTORY_SORT_H
