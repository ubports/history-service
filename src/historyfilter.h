#ifndef HISTORYFILTER_H
#define HISTORYFILTER_H

#include <QFlags>
#include <QSharedDataPointer>
#include <QVariant>

class HistoryFilterPrivate;

// simple filter
class HistoryFilter
{

public:
    enum MatchFlag {
        MatchCaseSensitive,
        MatchCaseInsensitive,
        MatchContains,
        MatchPhoneNumber
    };

    Q_DECLARE_FLAGS(MatchFlags, MatchFlag)

    HistoryFilter(const QString &filterProperty = QString::null,
                  const QVariant &filterValue = QVariant(),
                  MatchFlags matchFlags = MatchCaseSensitive);
    HistoryFilter(const HistoryFilter &other);
    virtual ~HistoryFilter();

    HistoryFilter &operator=(const HistoryFilter &other);

    QString filterProperty() const;
    void setFilterProperty(const QString &value);

    QVariant filterValue() const;
    void setFilterValue(const QVariant &value);

    MatchFlags matchFlags() const;
    void setMatchFlags(const MatchFlags &flags);
    virtual QString toString() const;

protected:
    HistoryFilter(HistoryFilterPrivate &p);
    QSharedDataPointer<HistoryFilterPrivate> d_ptr;

    // Q_DECLARE_PRIVATE equivalent for shared data pointers
    HistoryFilterPrivate* d_func();
    inline const HistoryFilterPrivate* d_func() const
    {
        return d_ptr.constData();
    }
};

#endif
