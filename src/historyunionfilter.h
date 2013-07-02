#ifndef HISTORYUNIONFILTER_H
#define HISTORYUNIONFILTER_H

#include <HistoryFilter>
#include <Types>

class HistoryUnionFilterPrivate;

// OR filter
class HistoryUnionFilter : public HistoryFilter
{

public:
    HistoryUnionFilter();
    HistoryUnionFilter(const HistoryUnionFilter &other);
    ~HistoryUnionFilter();

    void setFilters(const QList<HistoryFilter> &filters);
    void prepend(const HistoryFilter &filter);
    void append(const HistoryFilter &filter);

    QList<HistoryFilter> filters() const;
    QString toString() const;

protected:
    // Q_DECLARE_PRIVATE equivalent for shared data pointers
    HistoryUnionFilterPrivate *d_func();
    const HistoryUnionFilterPrivate *d_func() const;
};

#endif
