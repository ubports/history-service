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

    virtual ~HistoryItem();

    QString accountId() const;
    QString threadId() const;
    QString itemId() const;
    QString sender() const;
    QDateTime timestamp() const;
    bool newItem() const;
    virtual ItemType type() const = 0;

protected:
    HistoryItem(HistoryItemPrivate &p);
    QScopedPointer<HistoryItemPrivate> d_ptr;
};

#endif
