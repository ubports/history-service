#ifndef HISTORYUNIONFILTER_P_H
#define HISTORYUNIONFILTER_P_H

#include "historyfilter_p.h"

class HistoryUnionFilter;

class HistoryUnionFilterPrivate : public HistoryFilterPrivate
{

public:
    HistoryUnionFilterPrivate();
    ~HistoryUnionFilterPrivate();

    QList<HistoryFilterPtr> filters;
};

#endif // HISTORYUNIONFILTER_P_H
