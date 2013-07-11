#ifndef HISTORYEVENTMODEL_H
#define HISTORYEVENTMODEL_H

#include <QAbstractListModel>
#include "historythreadmodel.h"

class HistoryEventModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(HistoryQmlFilter *filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(HistoryThreadModel::EventType type READ type WRITE setType NOTIFY typeChanged)
    Q_ENUMS(Role)
public:
    enum Role {
        AccountIdRole = Qt::UserRole,
        ThreadIdRole,
        TypeRole,
        EventIdRole,
        SenderRole,
        TimestampRole,
        NewEventRole,
        TextMessageRole,
        TextMessageTypeRole,
        TextMessageFlagsRole,
        TextReadTimestampRole,
        CallMissedRole,
        CallDurationRole
    };

    explicit HistoryEventModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    QHash<int, QByteArray> roleNames() const;

    HistoryQmlFilter *filter() const;
    void setFilter(HistoryQmlFilter *value);

    HistoryThreadModel::EventType type() const;
    void setType(HistoryThreadModel::EventType value);

Q_SIGNALS:
    void filterChanged();
    void typeChanged();

protected Q_SLOTS:
    void updateQuery();

private:
    History::EventViewPtr mView;
    QList<History::EventPtr> mEvents;
    bool mCanFetchMore;
    HistoryQmlFilter *mFilter;
    HistoryThreadModel::EventType mType;
    QHash<int, QByteArray> mRoles;
};

#endif // HISTORYEVENTMODEL_H
