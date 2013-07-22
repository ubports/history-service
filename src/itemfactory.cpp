/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "itemfactory_p.h"
#include "itemfactory.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QDebug>

namespace History
{

QString ItemFactoryPrivate::hashItem(EventType type, const QString &accountId, const QString &threadId, const QString &eventId)
{
    return QString("item:%1:%2:%3:%4").arg(QString::number((int) type), accountId, threadId, eventId);
}

void ItemFactoryPrivate::cleanupThreads()
{
    QMap<QString, ThreadWeakPtr>::iterator it = threads.begin();
    QMap<QString, ThreadWeakPtr>::iterator end = threads.end();
    while (it != end) {
        if (it.value().isNull()) {
            it = threads.erase(it);
        } else {
            ++it;
        }
    }
}

void ItemFactoryPrivate::cleanupEvents()
{
    QMap<QString, EventWeakPtr>::iterator it = events.begin();
    QMap<QString, EventWeakPtr>::iterator end = events.end();
    while (it != end) {
        if (it.value().isNull()) {
            it = events.erase(it);
        } else {
            ++it;
        }
    }
}

ItemFactory::ItemFactory()
    : d_ptr(new ItemFactoryPrivate())
{
}

ItemFactory *ItemFactory::instance()
{
    static ItemFactory *self = new ItemFactory();
    return self;
}

ThreadPtr ItemFactory::createThread(const QString &accountId,
                                    const QString &threadId,
                                    EventType type,
                                    const QStringList &participants,
                                    const EventPtr &lastEvent,
                                    int count,
                                    int unreadCount)
{
    Q_D(ItemFactory);

    QString hash = d->hashItem(type, accountId, threadId);
    qDebug() << "Hash:" << hash;

    ThreadPtr thread;
    if (d->threads.contains(hash)) {
        thread = d->threads[hash].toStrongRef();
    }

    if (thread.isNull()) {
        thread = ThreadPtr(new Thread(accountId,
                                      threadId,
                                      type,
                                      participants,
                                      lastEvent,
                                      count,
                                      unreadCount));
        d->threads[hash] = thread;
    }

    // take the opportunity to clean the map
    d->cleanupThreads();

    return thread;
}

TextEventPtr ItemFactory::createTextEvent(const QString &accountId,
                                          const QString &threadId,
                                          const QString &eventId,
                                          const QString &senderId,
                                          const QDateTime &timestamp,
                                          bool newEvent,
                                          const QString &message,
                                          MessageType messageType,
                                          MessageFlags messageFlags,
                                          const QDateTime &readTimestamp)
{
    Q_D(ItemFactory);

    QString hash = d->hashItem(EventTypeText, accountId, threadId, eventId);
    qDebug() << "Hash:" << hash;

    EventPtr event;
    if (d->events.contains(hash)) {
        event = d->events[hash].toStrongRef();
    }

    if (event.isNull()) {
        event = TextEventPtr(new TextEvent(accountId,
                                           threadId,
                                           eventId,
                                           senderId,
                                           timestamp,
                                           newEvent,
                                           message,
                                           messageType,
                                           messageFlags,
                                           readTimestamp));
        d->events[hash] = event;
    }

    // take the opportunity to clean the map
    d->cleanupEvents();

    return event.staticCast<TextEvent>();
}

VoiceEventPtr ItemFactory::createVoiceEvent(const QString &accountId,
                                            const QString &threadId,
                                            const QString &eventId,
                                            const QString &senderId,
                                            const QDateTime &timestamp,
                                            bool newEvent,
                                            bool missed,
                                            const QTime &duration)
{
    Q_D(ItemFactory);

    QString hash = d->hashItem(EventTypeVoice, accountId, threadId, eventId);
    qDebug() << "Hash:" << hash;

    EventPtr event;
    if (d->events.contains(hash)) {
        event = d->events[hash].toStrongRef();
    }

    if (event.isNull()) {
        event = VoiceEventPtr(new VoiceEvent(accountId,
                                             threadId,
                                             eventId,
                                             senderId,
                                             timestamp,
                                             newEvent,
                                             missed,
                                             duration));
        d->events[hash] = event;
    }

    // take the opportunity to clean the map
    d->cleanupEvents();

    return event.staticCast<VoiceEvent>();
}

}
