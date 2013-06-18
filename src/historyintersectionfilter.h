// AND filter
class HistoryIntersectionFilter : public HistoryFilter
{
    Q_OBJECT
public:
    HistoryIntersectionFilter();
    ~HistoryIntersectionFilter();

    setFilters(const QList<HistoryFilter> &filters);
    prepend(const HistoryFilter &filter);
    append(const HistoryFilter &filter);

    QList<HistoryFilter> filters() const;
protected:
    virtual QString toString();
};

