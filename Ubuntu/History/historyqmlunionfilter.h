#ifndef HISTORYQMLUNIONFILTER_H
#define HISTORYQMLUNIONFILTER_H

#include "historyqmlfilter.h"

class HistoryQmlUnionFilter : public HistoryQmlCompoundFilter
{
    Q_OBJECT
public:
    explicit HistoryQmlUnionFilter(QObject *parent = 0);
    HistoryFilter filter() const;
};

#endif // HISTORYQMLUNIONFILTER_H
