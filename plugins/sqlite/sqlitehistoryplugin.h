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
#include <QSqlQuery>

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
    History::PluginThreadView* queryThreads(History::EventType type,
                                            const History::Sort &sort = History::Sort(),
                                            const History::Filter &filter = History::Filter());
    History::PluginEventView* queryEvents(History::EventType type,
                                          const History::Sort &sort = History::Sort(),
                                          const History::Filter &filter = History::Filter());
    QVariantMap threadForParticipants(const QString &accountId,
                                      History::EventType type,
                                      const QStringList &participants,
                                      History::MatchFlags matchFlags = History::MatchCaseSensitive);

    QList<QVariantMap> eventsForThread(const QVariantMap &thread);

    QVariantMap getSingleThread(History::EventType type, const QString &accountId, const QString &threadId);
    QVariantMap getSingleEvent(History::EventType type, const QString &accountId, const QString &threadId, const QString &eventId);

    // Writer part of the plugin
    QVariantMap createThreadForParticipants(const QString &accountId, History::EventType type, const QStringList &participants);
    bool removeThread(const QVariantMap &thread);

    History::EventWriteResult writeTextEvent(const QVariantMap &event);
    bool removeTextEvent(const QVariantMap &event);

    History::EventWriteResult writeVoiceEvent(const QVariantMap &event);
    bool removeVoiceEvent(const QVariantMap &event);

    bool beginBatchOperation();
    bool endBatchOperation();
    bool rollbackBatchOperation();

    // functions to be used internally
    QString sqlQueryForThreads(History::EventType type, const QString &condition, const QString &order);
    QList<QVariantMap> parseThreadResults(History::EventType type, QSqlQuery &query);

    QString sqlQueryForEvents(History::EventType type, const QString &condition, const QString &order);
    QList<QVariantMap> parseEventResults(History::EventType type, QSqlQuery &query);

    static QString toLocalTimeString(const QDateTime &timestamp);

    QString filterToString(const History::Filter &filter, const QString &propertyPrefix = QString::null) const;
};

#endif // SQLITEHISTORYPLUGIN_H
