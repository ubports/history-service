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

#ifndef HISTORY_VOICEEVENT_H
#define HISTORY_VOICEEVENT_H

#include "event.h"

namespace History
{

class VoiceEventPrivate;
class ItemFactory;

class VoiceEvent : public Event
{
    Q_DECLARE_PRIVATE(VoiceEvent)
    friend class ItemFactory;

public:
    explicit VoiceEvent();
    VoiceEvent(const QString &accountId,
               const QString &threadId,
               const QString &eventId,
               const QString &sender,
               const QDateTime &timestamp,
               bool newEvent,
               bool missed,
               const QTime &duration = QTime(),
               const QStringList &participants = QStringList());
    ~VoiceEvent();

    // copy related members
    VoiceEvent(const Event &other);
    VoiceEvent& operator=(const Event &other);

    bool missed() const;
    QTime duration() const;

    static Event fromProperties(const QVariantMap &properties);
};

}

#endif

