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

#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include <QObject>
#include <QSqlDatabase>

class SQLiteDatabase : public QObject
{
    Q_OBJECT
public:
    static SQLiteDatabase *instance();

    bool initializeDatabase();
    QSqlDatabase database() const;

    bool beginTransation();
    bool finishTransaction();
    bool rollbackTransaction();

    bool reopen();

    QString dumpSchema() const;
    QStringList parseSchemaFile(const QString &fileName);
    bool runMultipleStatements(const QStringList &statements, bool useTransaction = true);

protected:
    bool createOrUpdateDatabase();
    void parseVersionInfo();

    // data upgrade functions
    bool changeTimestampsToUtc();
    bool convertGroupChatToRoom();

private:
    explicit SQLiteDatabase(QObject *parent = 0);
    QString mDatabasePath;
    QSqlDatabase mDatabase;
    int mSchemaVersion;
    
};

#endif // SQLITEDATABASE_H
