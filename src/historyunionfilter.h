#ifndef HISTORYUNIONFILTER_H
#define HISTORYUNIONFILTER_H

#include <HistoryFilter>
#include <Types>

class HistoryUnionFilterPrivate;

// OR filter
class HistoryUnionFilter : public HistoryFilter
{
    Q_DECLARE_PRIVATE(HistoryUnionFilter)
public:
    HistoryUnionFilter();
    ~HistoryUnionFilter();

    void setFilters(const QList<HistoryFilterPtr> &filters);
    void prepend(const HistoryFilterPtr &filter);
    void append(const HistoryFilterPtr &filter);
    void clear();

    QList<HistoryFilterPtr> filters() const;
    QString toString() const;
};

#endif
