#ifndef HISTORYTHREADMODEL_H
#define HISTORYTHREADMODEL_H

#include <QAbstractListModel>
#include "types.h"

class HistoryQmlFilter;

class HistoryThreadModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(HistoryQmlFilter *filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(EventType type READ type WRITE setType NOTIFY typeChanged)
    Q_ENUMS(EventType)
    Q_ENUMS(Role)
public:
    enum EventType {
        EventTypeText = History::EventTypeText,
        EventTypeVoice = History::EventTypeVoice
    };

    enum Role {
        AccountIdRole = Qt::UserRole,
        ThreadIdRole,
        TypeRole,
        ParticipantsRole,
        CountRole,
        UnreadCountRole,
        LastEventIdRole,
        LastEventSenderRole,
        LastEventTimestampRole,
        LastEventNewRole,
        LastEventTextMessageRole,
        LastEventTextMessageTypeRole,
        LastEventTextMessageFlagsRole,
        LastEventTextReadTimestampRole,
        LastEventCallMissedRole,
        LastEventCallDurationRole
    };

    explicit HistoryThreadModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    QHash<int, QByteArray> roleNames() const;

    HistoryQmlFilter *filter() const;
    void setFilter(HistoryQmlFilter *value);

    EventType type() const;
    void setType(EventType value);

Q_SIGNALS:
    void filterChanged();
    void typeChanged();

protected Q_SLOTS:
    void updateQuery();

private:
    History::ThreadViewPtr mThreadView;
    QList<History::ThreadPtr> mThreads;
    bool mCanFetchMore;
    HistoryQmlFilter *mFilter;
    EventType mType;
    QHash<int, QByteArray> mRoles;
};

#endif // HISTORYTHREADMODEL_H
