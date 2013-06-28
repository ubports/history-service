#include "historyitem.h"
#include "historyitem_p.h"

// ------------- HistoryItemPrivate ------------------------------------------------
HistoryItemPrivate::HistoryItemPrivate()
{
}

HistoryItemPrivate::HistoryItemPrivate(const QString &theAccountId,
                                       const QString &theThreadId,
                                       const QString &theItemId,
                                       const QString &theSender,
                                       const QDateTime &theTimestamp,
                                       bool theNewItem) :
    accountId(theAccountId), threadId(theThreadId), itemId(theItemId),
    sender(theSender), timestamp(theTimestamp), newItem(theNewItem), q_ptr(0)
{
}

HistoryItemPrivate::~HistoryItemPrivate()
{
}

// ------------- HistoryItem -------------------------------------------------------

/*!
 * \class HistoryItem
 *
 * \brief The HistoryItem class provides the base class for all events stored
 *  and loaded from the history backends.
 *
 *  This class should not be used directly and instead
 *  the derived classes should be used.
 *
 * \sa TextItem, VoiceItem
 */

/*!
 * \fn HistoryItem::ItemType HistoryItem::type() const
 * \brief Returns the type of this event.
 */

/*!
  \internal
 * \brief Constructor to be used by derived classes to pass a HistoryItemPrivate instance
 * \param p The instance of the private class;
 */
HistoryItem::HistoryItem(HistoryItemPrivate &p)
    : d_ptr(&p)
{
    d_ptr->q_ptr = this;
}

HistoryItem::~HistoryItem()
{
}

/*!
 * \brief Returns the account ID this item belongs to.
 */
QString HistoryItem::accountId() const
{
    Q_D(const HistoryItem);
    return d->accountId;
}

/*!
 * \brief Returns the ID of the communication thread this item belongs to.
 * \sa HistoryThread
 */
QString HistoryItem::threadId() const
{
    Q_D(const HistoryItem);
    return d->threadId;
}

/*!
 * \brief Returns the ID that uniquely identifies this item.
 */
QString HistoryItem::itemId() const
{
    Q_D(const HistoryItem);
    return d->itemId;
}

/*!
 * \brief Returns the ID of the sender of this event.
 */
QString HistoryItem::sender() const
{
    Q_D(const HistoryItem);
    return d->sender;
}

/*!
 * \brief Returns the timestamp of when the event happened.
 */
QDateTime HistoryItem::timestamp() const
{
    Q_D(const HistoryItem);
    return d->timestamp;
}

/*!
 * \brief Returns whether  the item is new (not yet seen by the user).
 * \return
 */
bool HistoryItem::newItem() const
{
    Q_D(const HistoryItem);
    return d->newItem;
}
