#ifndef HISTORYQMLFILTER_H
#define HISTORYQMLFILTER_H

#include <qqml.h>
#include <QObject>
#include <HistoryFilter>
#include <Types>

class HistoryQmlFilter : public QObject
{
    Q_OBJECT
    Q_ENUMS(MatchFlag)
    Q_PROPERTY(QString filterProperty READ filterProperty WRITE setFilterProperty NOTIFY filterPropertyChanged)
    Q_PROPERTY(QVariant filterValue READ filterValue WRITE setFilterValue NOTIFY filterValueChanged)
    Q_PROPERTY(int matchFlags READ matchFlags WRITE setMatchFlags NOTIFY matchFlagsChanged)
public:
    enum MatchFlag {
        MatchCaseSensitive = HistoryFilter::MatchCaseSensitive,
        MatchCaseInsensitive = HistoryFilter::MatchCaseInsensitive,
        MatchContains = HistoryFilter::MatchContains,
        MatchPhoneNumber = HistoryFilter::MatchPhoneNumber
    };

    explicit HistoryQmlFilter(QObject *parent = 0);

    QString filterProperty() const;
    void setFilterProperty(const QString &value);

    QVariant filterValue() const;
    void setFilterValue(const QVariant &value);

    int matchFlags() const;
    void setMatchFlags(int flags);

    virtual HistoryFilterPtr filter() const;

Q_SIGNALS:
    void filterPropertyChanged();
    void filterValueChanged();
    void matchFlagsChanged();
    void filterChanged();
    
protected:
    HistoryFilterPtr mFilter;
};


// compound filter
class HistoryQmlCompoundFilter : public HistoryQmlFilter
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<HistoryQmlFilter> filters READ filters NOTIFY filtersChanged)
    Q_CLASSINFO("DefaultProperty", "filters")

public:
    explicit HistoryQmlCompoundFilter(QObject* parent = 0);
    virtual ~HistoryQmlCompoundFilter();
    QQmlListProperty<HistoryQmlFilter> filters();

    static void filtersAppend(QQmlListProperty<HistoryQmlFilter>* prop, HistoryQmlFilter* filter);
    static int filtersCount(QQmlListProperty<HistoryQmlFilter>* prop);
    static HistoryQmlFilter* filtersAt(QQmlListProperty<HistoryQmlFilter>* prop, int index);
    static void filtersClear(QQmlListProperty<HistoryQmlFilter>* prop);

Q_SIGNALS:
    void filtersChanged();

protected:
    QList<HistoryQmlFilter*> mFilters;
};

#endif // HISTORYQMLFILTER_H
