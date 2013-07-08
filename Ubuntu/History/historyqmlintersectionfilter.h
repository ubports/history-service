#ifndef HISTORYQMLINTERSECTIONFILTER_H
#define HISTORYQMLINTERSECTIONFILTER_H

#include "historyqmlfilter.h"

class HistoryQmlIntersectionFilter : public HistoryQmlCompoundFilter
{
    Q_OBJECT
public:
    explicit HistoryQmlIntersectionFilter(QObject *parent = 0);

    HistoryFilter filter() const;
};

#endif // HISTORYQMLINTERSECTIONFILTER_H
