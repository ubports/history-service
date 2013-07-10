#ifndef HISTORYITEMMODEL_H
#define HISTORYITEMMODEL_H

#include <QAbstractListModel>
#include "historythreadmodel.h"

class HistoryItemModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(HistoryQmlFilter *filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(HistoryThreadModel::ItemType type READ type WRITE setType NOTIFY typeChanged)
    Q_ENUMS(Role)
public:
    enum Role {
        AccountIdRole = Qt::UserRole,
        ThreadIdRole,
        TypeRole,
        ItemIdRole,
        SenderRole,
        TimestampRole,
        NewItemRole,
        TextMessageRole,
        TextMessageTypeRole,
        TextMessageFlagsRole,
        TextReadTimestampRole,
        CallMissedRole,
        CallDurationRole
    };

    explicit HistoryItemModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    QHash<int, QByteArray> roleNames() const;

    HistoryQmlFilter *filter() const;
    void setFilter(HistoryQmlFilter *value);

    HistoryThreadModel::ItemType type() const;
    void setType(HistoryThreadModel::ItemType value);

Q_SIGNALS:
    void filterChanged();
    void typeChanged();

protected Q_SLOTS:
    void updateQuery();

private:
    HistoryItemViewPtr mView;
    QList<HistoryItemPtr> mItems;
    bool mCanFetchMore;
    HistoryQmlFilter *mFilter;
    HistoryThreadModel::ItemType mType;
    QHash<int, QByteArray> mRoles;
};

#endif // HISTORYITEMMODEL_H
