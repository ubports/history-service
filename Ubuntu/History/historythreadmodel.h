#ifndef HISTORYTHREADMODEL_H
#define HISTORYTHREADMODEL_H

#include <QAbstractListModel>
#include <HistoryItem>
#include <Types>

class HistoryQmlFilter;

class HistoryThreadModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(HistoryQmlFilter *filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(ItemType type READ type WRITE setType NOTIFY typeChanged)
    Q_ENUMS(ItemType)
    Q_ENUMS(Role)
public:
    enum ItemType {
        ItemTypeText = HistoryItem::ItemTypeText,
        ItemTypeVoice = HistoryItem::ItemTypeVoice
    };

    enum Role {
        AccountIdRole = Qt::UserRole,
        ThreadIdRole,
        TypeRole,
        ParticipantsRole,
        CountRole,
        UnreadCountRole,
        LastItemIdRole,
        LastItemSenderRole,
        LastItemTimestampRole,
        LastItemNewRole,
        LastItemTextMessageRole,
        LastItemTextMessageTypeRole,
        LastItemTextMessageFlagsRole,
        LastItemTextReadTimestampRole,
        LastItemCallMissedRole,
        LastItemCallDurationRole
    };

    explicit HistoryThreadModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    QHash<int, QByteArray> roleNames() const;

    HistoryQmlFilter *filter() const;
    void setFilter(HistoryQmlFilter *value);

    ItemType type() const;
    void setType(ItemType value);

Q_SIGNALS:
    void filterChanged();
    void typeChanged();

protected Q_SLOTS:
    void updateQuery();

private:
    HistoryThreadViewPtr mThreadView;
    QList<HistoryThreadPtr> mThreads;
    bool mCanFetchMore;
    int mPageSize;
    HistoryQmlFilter *mFilter;
    ItemType mType;
    QHash<int, QByteArray> mRoles;
    
};

#endif // HISTORYTHREADMODEL_H
