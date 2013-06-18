// simple filter
class HistoryFilter : public QObject
{
    Q_OBJECT

    friend class HistoryManager;
public:
    enum MatchFlags {
        MatchCaseSensitive,
        MatchCaseInsensitive,
        MatchContains,
        MatchPhoneNumber
    };

    HistoryFilter(const QString &filterProperty = QString::null,
                  const QVariant &value = QVariant(),
                  MatchRules matchRules = MatchCaseSensitive);
    virtual ~HistoryFilter();

    QString filterProperty();
    void setFilterProperty(const QString &value);

    QVariant filterValue();
    void setFilterValue(const QVariant &value);

    MatchRules matchFlags();
    void setMatchFlags(MatchRules rules);

protected:
    virtual QString toString();
};

