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

#ifndef SQLITEHISTORYPLUGIN_H
#define SQLITEHISTORYPLUGIN_H

#include "plugin.h"
#include <QObject>

class SQLiteHistoryReader;
class SQLiteHistoryWriter;

typedef QSharedPointer<SQLiteHistoryReader> SQLiteHistoryReaderPtr;
typedef QSharedPointer<SQLiteHistoryWriter> SQLiteHistoryWriterPtr;

class SQLiteHistoryPlugin : public QObject, History::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.canonical.historyservice.Plugin")
    Q_INTERFACES(History::Plugin)
public:
    explicit SQLiteHistoryPlugin(QObject *parent = 0);

    // Reader part of the plugin
    History::ThreadViewPtr queryThreads(History::EventType type,
                                        const History::SortPtr &sort = History::SortPtr(),
                                        const History::FilterPtr &filter = History::FilterPtr());
    History::EventViewPtr queryEvents(History::EventType type,
                                      const History::SortPtr &sort = History::SortPtr(),
                                      const History::FilterPtr &filter = History::FilterPtr());
    History::ThreadPtr threadForParticipants(const QString &accountId,
                                             History::EventType type,
                                             const QStringList &participants,
                                             History::MatchFlags matchFlags = History::MatchCaseSensitive);

    History::ThreadPtr getSingleThread(History::EventType type, const QString &accountId, const QString &threadId);
    History::EventPtr getSingleEvent(History::EventType type, const QString &accountId, const QString &threadId, const QString &eventId);

    // Writer part of the plugin
    History::ThreadPtr createThreadForParticipants(const QString &accountId, History::EventType type, const QStringList &participants);
    bool removeThread(const History::ThreadPtr &thread);

    bool writeTextEvent(const History::TextEventPtr &event);
    bool removeTextEvent(const History::TextEventPtr &event);

    bool writeVoiceEvent(const History::VoiceEventPtr &event);
    bool removeVoiceEvent(const History::VoiceEventPtr &event);

    bool beginBatchOperation();
    bool endBatchOperation();
    bool rollbackBatchOperation();

};

#endif // SQLITEHISTORYPLUGIN_H
