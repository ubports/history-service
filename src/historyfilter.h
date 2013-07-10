#ifndef HISTORYFILTER_H
#define HISTORYFILTER_H

#include <QFlags>
#include <QScopedPointer>
#include <QVariant>

class HistoryFilterPrivate;

// simple filter
class HistoryFilter
{
    Q_DECLARE_PRIVATE(HistoryFilter)
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
    virtual ~HistoryFilter();

    QString filterProperty() const;
    void setFilterProperty(const QString &value);

    QVariant filterValue() const;
    void setFilterValue(const QVariant &value);

    MatchFlags matchFlags() const;
    void setMatchFlags(const MatchFlags &flags);
    virtual QString toString() const;

protected:
    HistoryFilter(HistoryFilterPrivate &p);
    QScopedPointer<HistoryFilterPrivate> d_ptr;
};

#endif
