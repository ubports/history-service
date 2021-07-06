/*
 * Copyright (C) 2013-2016 Canonical, Ltd.
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
#include "filter.h"
#include "types.h"
#include "sort.h"
#include <QVariantMap>

namespace History
{

class PluginThreadView;
class PluginEventView;

class Plugin
{
public:
    virtual ~Plugin() {}

    virtual bool initialised() { return true; }

    // Reader part of the plugin
    virtual PluginThreadView* queryThreads(EventType type,
                                       const Sort &sort = Sort(),
                                       const Filter &filter = Filter(),
                                       const QVariantMap &properties = QVariantMap()) = 0;
    virtual PluginEventView* queryEvents(EventType type,
                                         const Sort &sort = Sort(),
                                         const Filter &filter = Filter()) = 0;
    virtual int eventsCount(EventType type, const Filter &filter = Filter()) = 0;

    virtual QVariantMap getSingleThread(EventType type,
                                        const QString &accountId,
                                        const QString &threadId,
                                        const QVariantMap &properties = QVariantMap()) = 0;
    virtual QVariantMap getSingleEvent(EventType type,
                                       const QString &accountId,
                                       const QString &threadId,
                                       const QString &eventId) = 0;
    virtual QVariantMap threadForParticipants(const QString &accountId,
                                              EventType type,
                                              const QStringList &participants,
                                              History::MatchFlags matchFlags = History::MatchCaseSensitive) = 0;
    virtual QVariantMap threadForProperties(const QString &accountId,
                                              EventType type,
                                              const QVariantMap &properties,
                                              History::MatchFlags matchFlags = History::MatchCaseSensitive) = 0;
    virtual QString threadIdForProperties(const QString &accountId,
                                          EventType type,
                                          const QVariantMap &properties,
                                          History::MatchFlags matchFlags = History::MatchCaseSensitive) = 0;
    virtual QList<QVariantMap> participantsForThreads(const QList<QVariantMap> &threadIds) = 0;

    virtual QList<QVariantMap> eventsForThread(const QVariantMap &thread) = 0;

    // Writer part of the plugin
    virtual QVariantMap createThreadForParticipants(const QString& /* accountId */, EventType /* type */, const QStringList& /* participants */) { return QVariantMap(); }
    virtual QVariantMap createThreadForProperties(const QString& /* accountId */, EventType /* type */, const QVariantMap& /* properties */) { return QVariantMap(); }
    virtual bool updateRoomParticipants(const QString& /* accountId */, const QString& /* threadId */, History::EventType /* type */, const QVariantList& /* participants */) { return false; };
    virtual bool updateRoomParticipantsRoles(const QString& /* accountId */, const QString& /* threadId */, History::EventType /* type */, const QVariantMap& /* participantsRoles */) { return false; };
    virtual bool updateRoomInfo(const QString& /* accountId */, const QString& /* threadId */, EventType /* type */, const QVariantMap& /* properties */, const QStringList& /* invalidated */ = QStringList()) { return false; };
    virtual bool removeThread(const QVariantMap& /* thread */) { return false; }
    virtual QVariantMap markThreadAsRead(const QVariantMap& /* thread */) { return QVariantMap(); }

    virtual EventWriteResult writeTextEvent(const QVariantMap& /* event */) { return EventWriteError; }
    virtual bool removeTextEvent(const QVariantMap& /* event */) { return false; }

    virtual EventWriteResult writeVoiceEvent(const QVariantMap& /* event */) { return EventWriteError; }
    virtual bool removeVoiceEvent(const QVariantMap& /* event */) { return false; }

    virtual bool beginBatchOperation() { return false; }
    virtual bool endBatchOperation() { return false; }
    virtual bool rollbackBatchOperation() { return false; }

    // FIXME: this is hackish, but changing it required a broad refactory of HistoryDaemon
    virtual void generateContactCache() {}
};

}

Q_DECLARE_INTERFACE(History::Plugin, "com.canonical.historyservice.Plugin")

#endif // HISTORY_PLUGIN_H
