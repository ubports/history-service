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

#include "phoneutils_p.h"
#include "sqlite3.h"
#include "sqlitedatabase.h"
#include "types.h"
#include "utils_p.h"
#include <QStandardPaths>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>

Q_DECLARE_OPAQUE_POINTER(sqlite3*)
Q_DECLARE_METATYPE(sqlite3*)

// custom sqlite function "comparePhoneNumbers" used to compare IDs if necessary
void comparePhoneNumbers(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    QString arg1((const char*)sqlite3_value_text(argv[0]));
    QString arg2((const char*)sqlite3_value_text(argv[1]));
    sqlite3_result_int(context, (int)PhoneUtils::comparePhoneNumbers(arg1, arg2));
}

void compareNormalizedPhoneNumbers(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    QString arg1((const char*)sqlite3_value_text(argv[0]));
    QString arg2((const char*)sqlite3_value_text(argv[1]));
    sqlite3_result_int(context, (int)PhoneUtils::compareNormalizedPhoneNumbers(arg1, arg2));
}

void normalizeId(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    QString accountId((const char*)sqlite3_value_text(argv[0]));
    QString id((const char*)sqlite3_value_text(argv[1]));
    QString normalizedId = id;
    // for now we only normalize phone number IDs
    if (History::Utils::matchFlagsForAccount(accountId) & History::MatchPhoneNumber) {
        normalizedId = PhoneUtils::normalizePhoneNumber(id);
    }
    sqlite3_result_text(context, normalizedId.toUtf8().data(), -1, NULL);
}

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
    mDatabasePath = qgetenv("HISTORY_SQLITE_DBPATH");

    if (mDatabasePath.isEmpty()) {
        mDatabasePath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

        QDir dir(mDatabasePath);
        if (!dir.exists("history-service") && !dir.mkpath("history-service")) {
            qCritical() << "Failed to create dir";
            return false;
        }
        dir.cd("history-service");

        mDatabasePath = dir.absoluteFilePath("history.sqlite");
    }

    mDatabase = QSqlDatabase::addDatabase("QSQLITE");
    mDatabase.setDatabaseName(mDatabasePath);

    qDebug() << "Using database at" << mDatabasePath;

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

/// this method is to be used mainly by unit tests in order to clean up the database between
/// tests.
bool SQLiteDatabase::reopen()
{
    mDatabase.close();
    mDatabase.open();

    // make sure the database is up-to-date after reopening.
    // this is mainly required for the memory backend used for testing
    createOrUpdateDatabase();
}

bool SQLiteDatabase::createOrUpdateDatabase()
{
    bool create = !QFile(mDatabasePath).exists();

    if (!mDatabase.open()) {
        return false;
    }

    // create the comparePhoneNumbers custom sqlite functions
    sqlite3 *handle = database().driver()->handle().value<sqlite3*>();
    sqlite3_create_function(handle, "comparePhoneNumbers", 2, SQLITE_ANY, NULL, &comparePhoneNumbers, NULL, NULL);
    sqlite3_create_function(handle, "compareNormalizedPhoneNumbers", 2, SQLITE_ANY, NULL, &compareNormalizedPhoneNumbers, NULL, NULL);

    // and also create the normalizeId function
    sqlite3_create_function(handle, "normalizeId", 2, SQLITE_ANY, NULL, &normalizeId, NULL, NULL);

    parseVersionInfo();

    QSqlQuery query(mDatabase);
    // use memory to create temporary tables
    query.exec("PRAGMA temp_store = MEMORY");

    QStringList statements;
    int existingVersion = 0;

    if (create) {
         statements = parseSchemaFile(":/database/schema/schema.sql");
    } else {
        // if the database already exists, we don´t need to create the tables
        // only check if an update is needed
        query.exec("SELECT * FROM schema_version");
        if (!query.exec() || !query.next()) {
            return false;
        }

        existingVersion = query.value(0).toInt();
        int upgradeToVersion = existingVersion + 1;
        while (upgradeToVersion <= mSchemaVersion) {
            statements += parseSchemaFile(QString(":/database/schema/v%1.sql").arg(QString::number(upgradeToVersion)));
            ++upgradeToVersion;
        }
    }

    // if at this point needsUpdate is still false, it means the database is up-to-date
    if (statements.isEmpty()) {
        return true;
    }

    beginTransation();

    Q_FOREACH(const QString &statement, statements) {
        if (!query.exec(statement)) {
            qCritical() << "Failed to create or update database. SQL Statements:" << query.lastQuery() << "Error:" << query.lastError();
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

    // now check if any data updating is required
    if (existingVersion > 0) {
        // v10 - timestamps in UTC
        if (existingVersion > 0 && existingVersion < 10) {
            if (!changeTimestampsToUtc()) {
                qCritical() << "Failed to update existing data.";
                rollbackTransaction();
                return false;
            }
        }
    }

    finishTransaction();

    return true;
}

QStringList SQLiteDatabase::parseSchemaFile(const QString &fileName)
{
    QFile schema(fileName);
    if (!schema.open(QFile::ReadOnly)) {
        qCritical() << "Failed to open " << fileName;
        return QStringList();
    }

    bool parsingBlock = false;
    QString statement;
    QStringList statements;

    // FIXME: this parser is very basic, it needs to be improved in the future
    //        it does a lot of assumptions based on the structure of the schema.sql file

    QTextStream stream(&schema);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        bool statementEnded = false;

        statement += line;

        // check if we are parsing a trigger command
        if (line.trimmed().startsWith("CREATE TRIGGER", Qt::CaseInsensitive)) {
            parsingBlock = true;
        } else if (parsingBlock) {
            if (line.contains("END;")) {
                parsingBlock = false;
                statementEnded = true;
            }
        } else if (statement.contains(";")) {
            statementEnded = true;
        }

        statement += "\n";

        if (statementEnded) {
            statements.append(statement);
            statement.clear();
        }
    }

    return statements;
}

void SQLiteDatabase::parseVersionInfo()
{
    QFile schema(":/database/schema/version.info");
    if (!schema.open(QFile::ReadOnly)) {
        qDebug() << schema.error();
        qCritical() << "Failed to get database version";
    }

    QString version = schema.readAll();
    mSchemaVersion = version.toInt();
}

bool SQLiteDatabase::changeTimestampsToUtc()
{
    // update the text events
    QSqlQuery query(database());
    QString queryText = "SELECT accountId, threadId, eventId, timestamp, readTimestamp FROM text_events";

    if (!query.exec(queryText)) {
        qWarning() << "Failed to update text events:" << query.lastError();
        return false;
    }

    QList<QVariantMap> events;
    while (query.next()) {
        QVariantMap event;
        event[History::FieldAccountId] = query.value(0);
        event[History::FieldThreadId] = query.value(1);
        event[History::FieldEventId] = query.value(2);
        event[History::FieldTimestamp] = query.value(3);
        event[History::FieldReadTimestamp] = query.value(4);
        events << event;
    }
    query.clear();
    queryText = "UPDATE text_events SET timestamp=:timestamp, readTimestamp=:readTimestamp";
    queryText += " WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId";
    query.prepare(queryText);
    Q_FOREACH (const QVariantMap &event, events) {
        query.bindValue(":accountId", event[History::FieldAccountId]);
        query.bindValue(":threadId", event[History::FieldThreadId]);
        query.bindValue(":eventId", event[History::FieldEventId]);
        query.bindValue(":timestamp", event[History::FieldTimestamp].toDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ss.zzz"));
        query.bindValue(":readTimestamp", event[History::FieldReadTimestamp].toDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ss.zzz"));
        if (!query.exec()) {
            qWarning() << "Failed to update text event:" << query.lastError();
            return false;
        }
    }

    // and do the same for voice events
    queryText = "SELECT accountId, threadId, eventId, timestamp FROM voice_events";

    if (!query.exec(queryText)) {
        qWarning() << "Failed to update voice events:" << query.lastError();
        return false;
    }

    events.clear();
    while (query.next()) {
        QVariantMap event;
        event[History::FieldAccountId] = query.value(0);
        event[History::FieldThreadId] = query.value(1);
        event[History::FieldEventId] = query.value(2);
        event[History::FieldTimestamp] = query.value(3);
        events << event;
    }
    query.clear();
    queryText = "UPDATE voice_events SET timestamp=:timestamp";
    queryText += " WHERE accountId=:accountId AND threadId=:threadId AND eventId=:eventId";
    query.prepare(queryText);
    Q_FOREACH (const QVariantMap &event, events) {
        query.bindValue(":accountId", event[History::FieldAccountId]);
        query.bindValue(":threadId", event[History::FieldThreadId]);
        query.bindValue(":eventId", event[History::FieldEventId]);
        query.bindValue(":timestamp", event[History::FieldTimestamp].toDateTime().toUTC().toString("yyyy-MM-ddTHH:mm:ss.zzz"));
        if (!query.exec()) {
            qWarning() << "Failed to update voice event:" << query.lastError();
            return false;
        }
    }

    return true;
}
