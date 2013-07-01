#ifndef HISTORYTHREAD_H
#define HISTORYTHREAD_H

#include <Types>
#include <HistoryItem>
#include <QDateTime>
#include <QScopedPointer>
#include <QStringList>

class HistoryThreadPrivate;

class HistoryThread
{
    Q_DECLARE_PRIVATE(HistoryThread)

public:
    HistoryThread();
    HistoryThread(const QString &accountId,
                  const QString &threadId,
                  HistoryItem::ItemType type,
                  const QStringList &participants,
                  const HistoryItemPtr &lastItem = HistoryItemPtr(),
                  int count = 0,
                  int unreadCount = 0);
    ~HistoryThread();

    QString accountId() const;
    QString threadId() const;
    HistoryItem::ItemType type() const;
    QStringList participants() const;
    HistoryItemPtr lastItem() const;
    int count() const;
    int unreadCount() const;

protected:
    QScopedPointer<HistoryThreadPrivate> d_ptr;
};

#endif
