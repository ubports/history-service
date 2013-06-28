#ifndef HISTORYFILTER_P_H
#define HISTORYFILTER_P_H

#include "historyfilter.h"
#include <QString>
#include <QVariant>

class HistoryFilterPrivate
{
    Q_DECLARE_PUBLIC(HistoryFilter)

public:
    HistoryFilterPrivate();
    HistoryFilterPrivate(const QString &theFilterProperty,
                         const QVariant &theFilterValue,
                         HistoryFilter::MatchFlags theMatchFlags);
    virtual ~HistoryFilterPrivate();


    QString filterProperty;
    QVariant filterValue;
    HistoryFilter::MatchFlags matchFlags;

    HistoryFilter *q_ptr;
};

#endif // HISTORYFILTER_P_H
