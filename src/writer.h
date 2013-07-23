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

#ifndef HISTORYWRITER_H
#define HISTORYWRITER_H

#include "types.h"

namespace History
{

class Writer : public QObject
{
    Q_OBJECT
public:
    explicit Writer(QObject *parent = 0) : QObject(parent) {}
    virtual ~Writer() {}

    virtual ThreadPtr createThreadForParticipants(const QString &accountId, EventType type, const QStringList &participants) = 0;
    virtual bool writeTextEvent(const TextEventPtr &event) = 0;
    virtual bool writeVoiceEvent(const VoiceEventPtr &event) = 0;

    virtual bool beginBatchOperation() {}
    virtual bool endBatchOperation() {}

    // TODO: check if there is the need to write MMS entries
};

}

#endif
