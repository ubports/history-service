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

#include "sqlitedatabase.h"
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QDir>

SQLiteDatabase::SQLiteDatabase(QObject *parent) :
    QObject(parent), mSchemaVersion(0)
{
    initializeDatabase();
}


SQLiteDatabase *SQLiteDatabase::instance()
{
    static SQLiteDatabase *self = new SQLiteDatabase();
    return self;
}

bool SQLiteDatabase::initializeDatabase()
{
    mDatabasePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    QDir dir(mDatabasePath);
    qDebug() << "DatabasePath:" << dir.absolutePath();
    if (!dir.exists("history-service") && !dir.mkpath("history-service")) {
        qDebug() << "Failed to create dir";
        return false;
    }
    dir.cd("history-service");

    mDatabasePath = dir.absoluteFilePath("history.sqlite");
    qDebug() << "History database:" << mDatabasePath;

    mDatabase = QSqlDatabase::addDatabase("QSQLITE");
    mDatabase.setDatabaseName(mDatabasePath);

    // always run the createDatabase function at least during the development
    if (!createOrUpdateDatabase()) {
        qCritical() << "Failed to create or update the database";
        return false;
    }

    return true;
}

QSqlDatabase SQLiteDatabase::database() const
{
    return mDatabase;
}

bool SQLiteDatabase::beginTransation()
{
    return mDatabase.transaction();
}

bool SQLiteDatabase::finishTransaction()
{
    return mDatabase.commit();
}

bool SQLiteDatabase::rollbackTransaction()
{
    return mDatabase.rollback();
}

bool SQLiteDatabase::createOrUpdateDatabase()
{
    qDebug() << __PRETTY_FUNCTION__;

    bool needsUpdate = !QFile(mDatabasePath).exists();
    QStringList statements = parseSchema();

    if (!mDatabase.open()) {
        return false;
    }

    QSqlQuery query(mDatabase);

    // at this point if needsUpdate is false it means the database already exists
    // but we still need to check its schema version
    if (!needsUpdate) {
        query.exec("SELECT * FROM schema_version");
        if (!query.exec() || query.next() && query.value(0).toInt() < mSchemaVersion) {
            needsUpdate = true;
        }
    }

    // if at this point needsUpdate is still false, it means the database is up-to-date
    if (!needsUpdate) {
        qDebug() << "Database up-to-date. No schema update needed.";
        return true;
    }

    beginTransation();

    Q_FOREACH(QString statement, statements) {
        if (statement.trimmed().isEmpty()) {
            continue;
        }

        if (!query.exec(statement.remove("#"))) {
            qCritical() << "Failed to create table. SQL Statement:" << query.lastQuery() << "Error:" << query.lastError();
            rollbackTransaction();
            return false;
        }
    }

    // now set the new database schema version
    if (!query.exec("DELETE FROM schema_version")) {
        qCritical() << "Failed to remove previous schema versions. SQL Statement:" << query.lastQuery() << "Error:" << query.lastError();
        rollbackTransaction();
        return false;
    }

    if (!query.exec(QString("INSERT INTO schema_version VALUES (%1)").arg(mSchemaVersion))) {
        qCritical() << "Failed to insert new schema version. SQL Statement:" << query.lastQuery() << "Error:" << query.lastError();
        rollbackTransaction();
        return false;
    }

    finishTransaction();

    return true;
}

QStringList SQLiteDatabase::parseSchema()
{
    QFile schema(":/database/schema.sql");
    if (!schema.open(QFile::ReadOnly)) {
        return QStringList();
    }

    QTextStream stream(&schema);

    QStringList lines;
    while (!stream.atEnd()) {
        QString line = stream.readLine();

        // strip out comments
        if (line.trimmed().startsWith("//")) {
            if (line.indexOf("SCHEMA_VERSION") >= 0) {
                mSchemaVersion = line.split("=")[1].toInt();
                qDebug() << "Got schema version:" << mSchemaVersion;
            }
            continue;
        }
        lines << line;
    }
    return lines.join(" ").split("#");
}
