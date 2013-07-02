#ifndef HISTORYINTERSECTIONFILTER_H
#define HISTORYINTERSECTIONFILTER_H

#include <HistoryFilter>
#include <Types>

class HistoryIntersectionFilterPrivate;

// AND filter
class HistoryIntersectionFilter : public HistoryFilter
{

public:
    HistoryIntersectionFilter();
    HistoryIntersectionFilter(const HistoryIntersectionFilter &other);
    ~HistoryIntersectionFilter();

    void setFilters(const QList<HistoryFilter> &filters);
    void prepend(const HistoryFilter &filter);
    void append(const HistoryFilter &filter);

    QList<HistoryFilter> filters() const;
    QString toString() const;

protected:
    // Q_DECLARE_PRIVATE equivalent for shared data pointers
    HistoryIntersectionFilterPrivate *d_func();
    const HistoryIntersectionFilterPrivate *d_func() const;
};

#endif
