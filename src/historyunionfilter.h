// OR filter
class HistoryUnionFilter : public HistoryFilter
{
    Q_OBJECT
public:
    HistoryUnionFilter();
    ~HistoryUnionFilter();

    setFilters(const QList<HistoryFilter> &filters);
    prepend(const HistoryFilter &filter);
    append(const HistoryFilter &filter);

    QList<HistoryFilter> filters() const;
protected:
    virtual QString toString();
};

