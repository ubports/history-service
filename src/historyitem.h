
#ifndef HISTORYITEM_H
#define HISTORYITEM_H

#include <QDateTime>
#include <QScopedPointer>
#include <QString>

class HistoryItemPrivate;

class HistoryItem
{
    Q_DECLARE_PRIVATE(HistoryItem)

public:
    enum ItemType {
        TextItem,
        VoiceItem
    };

    HistoryItem();
    HistoryItem(const QString accountId,
                const QString &threadId,
                const QString &itemId,
                const QString &sender,
                const QDateTime &timestamp,
                ItemType type);
    virtual ~HistoryItem();

    QString accountId() const;
    QString threadId() const;
    QString itemId() const;
    QString sender() const;
    QDateTime timestamp() const;
    ItemType type() const;

protected:
    QScopedPointer<HistoryItemPrivate> d_ptr;
};

#endif
