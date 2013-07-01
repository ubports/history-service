#ifndef HISTORYTHREAD_P_H
#define HISTORYTHREAD_P_H

#include <QString>
#include <Types>

class HistoryThread;

class HistoryThreadPrivate
{
    Q_DECLARE_PUBLIC(HistoryThread)
public:
    HistoryThreadPrivate();
    HistoryThreadPrivate(const QString &theAccountId,
                         const QString &theThreadId,
                         HistoryItem::ItemType theType,
                         const QStringList &theParticipants,
                         const HistoryItemPtr &theLastItem,
                         int theCount,
                         int theUnreadCount);
    virtual ~HistoryThreadPrivate();

    QString accountId;
    QString threadId;
    QStringList participants;
    HistoryItem::ItemType type;
    HistoryItemPtr lastItem;
    int count;
    int unreadCount;

    HistoryThread *q_ptr;
};


#endif // HISTORYTHREAD_P_H
