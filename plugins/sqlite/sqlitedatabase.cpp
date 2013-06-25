#include "sqlitedatabase.h"
#include <QStandardPaths>
#include <QSqlQuery>
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
    mDatabasePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir dir(mDatabasePath);
    qDebug() << "DatabasePath:" << mDatabasePath;
    if (!dir.exists("history") && !dir.mkpath("history")) {
        qDebug() << "Failed to create dir";
        return false;
    }
    dir.cd("history");

    mDatabasePath = dir.absoluteFilePath("history.sqlite");
    qDebug() << "History database:" << mDatabasePath;

    mDatabase = QSqlDatabase::addDatabase("QSQLITE");
    mDatabase.setDatabaseName(mDatabasePath);

    if (!QFile(mDatabasePath).exists() && !createDatabase()) {
        return false;
    } else if (!mDatabase.open()) {
        return false;
    }

    // TODO: verify if the database structure is correct
    return true;
}

QSqlDatabase SQLiteDatabase::database() const
{
    return mDatabase;
}

bool SQLiteDatabase::createDatabase()
{
    if (!mDatabase.open()) {
        return false;
    }

    QStringList fields;
    fields << "accountId varchar(255)"
           << "threadId varchar(255)"
           << "type tinyint"
           << "lastItemId varchar(255)"
           << "count int"
           << "unreadCount int";

    QSqlQuery query(QString("CREATE TABLE threads (%1)").arg(fields.join(",")), mDatabase);
    if (!query.exec()) {
        return false;
    }

    fields.clear();
    fields << "accoundId varchar(255)"
           << "threadId varchar(255)"
           << "type tinyint"
           << "participandId varchar(255)";

    query = QSqlQuery(QString("CREATE TABLE thread_participants (%1)").arg(fields.join(",")), mDatabase);
    if (!query.exec()) {
        qCritical() << "Failed to create thread_participants table.";
        return false;
    }

    mDatabase.commit();
}
