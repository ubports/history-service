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
    QObject(parent)
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

    if (!QFile(mDatabasePath).exists() && !createDatabase()) {
        qCritical() << "Failed to create the database";
        return false;
    } else if (!mDatabase.open()) {
        qCritical() << "Failed to open the database";
        return false;
    }

    qDebug() << "Successfully opened the database. Ready to start logging.";

    // TODO: verify if the database structure is correct
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

bool SQLiteDatabase::createDatabase()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!mDatabase.open()) {
        return false;
    }

    QFile schema(":/database/schema.sql");
    if (!schema.open(QFile::ReadOnly)) {
        return false;
    }

    QTextStream stream(&schema);
    QSqlQuery query(mDatabase);
    QStringList statements = stream.readAll().split("#");

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

    finishTransaction();

    return true;
}
