#ifndef HISTORYFILTER_H
#define HISTORYFILTER_H

#include <QFlags>
#include <QObject>
#include <QScopedPointer>
#include <QVariant>

class HistoryFilterPrivate;

// simple filter
class HistoryFilter : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(HistoryFilter)

    friend class HistoryManager;

public:
    enum MatchFlag {
        MatchCaseSensitive,
        MatchCaseInsensitive,
        MatchContains,
        MatchPhoneNumber
    };
    typedef QFlags<MatchFlag> MatchFlags;

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

protected:
    virtual QString toString();

    HistoryFilter(HistoryFilterPrivate &p);
    QScopedPointer<HistoryFilterPrivate> d_ptr;
};

#endif
