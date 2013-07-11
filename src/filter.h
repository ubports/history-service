#ifndef HISTORY_FILTER_H
#define HISTORY_FILTER_H

#include <QFlags>
#include <QScopedPointer>
#include <QVariant>
#include "types.h"

namespace History
{

class FilterPrivate;

// simple filter
class Filter
{
    Q_DECLARE_PRIVATE(Filter)
public:
    Filter(const QString &filterProperty = QString::null,
                  const QVariant &filterValue = QVariant(),
                  MatchFlags matchFlags = MatchCaseSensitive);
    virtual ~Filter();

    QString filterProperty() const;
    void setFilterProperty(const QString &value);

    QVariant filterValue() const;
    void setFilterValue(const QVariant &value);

    MatchFlags matchFlags() const;
    void setMatchFlags(const MatchFlags &flags);
    virtual QString toString() const;

protected:
    Filter(FilterPrivate &p);
    QScopedPointer<FilterPrivate> d_ptr;
};

}

#endif
