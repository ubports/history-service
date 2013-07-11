#ifndef HISTORYQMLUNIONFILTER_H
#define HISTORYQMLUNIONFILTER_H

#include "historyqmlfilter.h"
#include <Types>

class HistoryQmlUnionFilter : public HistoryQmlCompoundFilter
{
    Q_OBJECT
public:
    explicit HistoryQmlUnionFilter(QObject *parent = 0);
    History::FilterPtr filter() const;
private:
    History::UnionFilterPtr mFilter;
};

#endif // HISTORYQMLUNIONFILTER_H
