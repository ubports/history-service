#ifndef HISTORY_FILTER_P_H
#define HISTORY_FILTER_P_H

#include <QString>
#include <QVariant>
#include "types.h"

namespace History
{

class FilterPrivate
{

public:
    FilterPrivate();
    FilterPrivate(const QString &theFilterProperty,
                         const QVariant &theFilterValue,
                         MatchFlags theMatchFlags);
    virtual ~FilterPrivate();


    QString filterProperty;
    QVariant filterValue;
    MatchFlags matchFlags;
};

}

#endif // HISTORY_FILTER_P_H
