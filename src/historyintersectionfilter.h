#ifndef HISTORYINTERSECTIONFILTER_H
#define HISTORYINTERSECTIONFILTER_H

#include <HistoryFilter>
#include <Types>

class HistoryIntersectionFilterPrivate;

// AND filter
class HistoryIntersectionFilter : public HistoryFilter
{
    Q_DECLARE_PRIVATE(HistoryIntersectionFilter)
public:
    HistoryIntersectionFilter();
    ~HistoryIntersectionFilter();

    void setFilters(const QList<HistoryFilterPtr> &filters);
    void prepend(const HistoryFilterPtr &filter);
    void append(const HistoryFilterPtr &filter);
    void clear();

    QList<HistoryFilterPtr> filters() const;
    QString toString() const;
};

#endif
