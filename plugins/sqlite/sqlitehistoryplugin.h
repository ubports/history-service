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

    History::WriterPtr writer() const;
    History::ReaderPtr reader() const;

private:
    SQLiteHistoryReaderPtr mReader;
    SQLiteHistoryWriterPtr mWriter;
};

#endif // SQLITEHISTORYPLUGIN_H
