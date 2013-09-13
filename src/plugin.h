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

#ifndef HISTORY_PLUGIN_H
#define HISTORY_PLUGIN_H

#include <QtPlugin>
#include "types.h"

namespace History
{

class PluginThreadView;

class Plugin
{
public:
    virtual ~Plugin() {}

    // Reader part of the plugin
    virtual PluginThreadView* queryThreads(EventType type,
                                       const SortPtr &sort = SortPtr(),
                                       const QString &filter = QString::null) = 0;
    virtual EventViewPtr queryEvents(EventType type,
                                     const SortPtr &sort = SortPtr(),
                                     const FilterPtr &filter = FilterPtr()) = 0;
    virtual ThreadPtr getSingleThread(EventType type,
                                      const QString &accountId,
                                      const QString &threadId) = 0;
    virtual EventPtr getSingleEvent(EventType type,
                                    const QString &accountId,
                                    const QString &threadId,
                                    const QString &eventId) = 0;
    virtual ThreadPtr threadForParticipants(const QString &accountId,
                                            EventType type,
                                            const QStringList &participants,
                                            History::MatchFlags matchFlags = History::MatchCaseSensitive) = 0;

    // Writer part of the plugin
    virtual ThreadPtr createThreadForParticipants(const QString &accountId, EventType type, const QStringList &participants) { return ThreadPtr(); }
    virtual bool removeThread(const History::ThreadPtr &thread) { return false; }

    virtual bool writeTextEvent(const TextEventPtr &event) { return false; }
    virtual bool removeTextEvent(const History::TextEventPtr &event) { return false; }

    virtual bool writeVoiceEvent(const VoiceEventPtr &event) { return false; }
    virtual bool removeVoiceEvent(const History::VoiceEventPtr &event) { return false; }

    virtual bool beginBatchOperation() {}
    virtual bool endBatchOperation() {}
    virtual bool rollbackBatchOperation() {}
};

}

Q_DECLARE_INTERFACE(History::Plugin, "com.canonical.historyservice.Plugin")

#endif // HISTORY_PLUGIN_H
