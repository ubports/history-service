#include "event.h"
#include "event_p.h"

namespace History
{

// ------------- EventPrivate ------------------------------------------------
EventPrivate::EventPrivate()
{
}

EventPrivate::EventPrivate(const QString &theAccountId,
                                       const QString &theThreadId,
                                       const QString &theItemId,
                                       const QString &theSender,
                                       const QDateTime &theTimestamp,
                                       bool theNewItem) :
    accountId(theAccountId), threadId(theThreadId), eventId(theItemId),
    sender(theSender), timestamp(theTimestamp), newEvent(theNewItem)
{
}

EventPrivate::~EventPrivate()
{
}

// ------------- Event -------------------------------------------------------

/*!
 * \class Event
 *
 * \brief The Event class provides the base class for all events stored
 *  and loaded from the history backends.
 *
 *  This class should not be used directly and instead
 *  the derived classes should be used.
 *
 * \sa TextItem, VoiceItem
 */

/*!
 * \fn Event::EventType Event::type() const
 * \brief Returns the type of this event.
 */

/*!
  \internal
 * \brief Constructor to be used by derived classes to pass a EventPrivate instance
 * \param p The instance of the private class;
 */
Event::Event(EventPrivate &p)
    : d_ptr(&p)
{
}

Event::~Event()
{
}

/*!
 * \brief Returns the account ID this item belongs to.
 */
QString Event::accountId() const
{
    Q_D(const Event);
    return d->accountId;
}

/*!
 * \brief Returns the ID of the communication thread this item belongs to.
 * \sa HistoryThread
 */
QString Event::threadId() const
{
    Q_D(const Event);
    return d->threadId;
}

/*!
 * \brief Returns the ID that uniquely identifies this event.
 */
QString Event::eventId() const
{
    Q_D(const Event);
    return d->eventId;
}

/*!
 * \brief Returns the ID of the sender of this event.
 */
QString Event::sender() const
{
    Q_D(const Event);
    return d->sender;
}

/*!
 * \brief Returns the timestamp of when the event happened.
 */
QDateTime Event::timestamp() const
{
    Q_D(const Event);
    return d->timestamp;
}

/*!
 * \brief Returns whether the event is new (not yet seen by the user).
 * \return
 */
bool Event::newEvent() const
{
    Q_D(const Event);
    return d->newEvent;
}

}
