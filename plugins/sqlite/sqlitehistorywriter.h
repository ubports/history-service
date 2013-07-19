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

#ifndef SQLITEHISTORYWRITER_H
#define SQLITEHISTORYWRITER_H

#include <QStringList>
#include "writer.h"
#include "types.h"

class SQLiteHistoryWriter : public History::Writer
{
    Q_OBJECT
public:
    explicit SQLiteHistoryWriter(QObject *parent = 0);

    History::ThreadPtr createThreadForParticipants(const QString &accountId, History::EventType type, const QStringList &participants);
    bool writeTextEvent(const History::TextEventPtr &event);
    bool writeVoiceEvent(const History::VoiceEventPtr &event);

    bool beginBatchOperation();
    bool endBatchOperation();
};

#endif // SQLITEHISTORYWRITER_H
