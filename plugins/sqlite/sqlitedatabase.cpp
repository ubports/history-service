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

    Q_FOREACH(QString statement, statements) {
        if (statement.trimmed().isEmpty()) {
            continue;
        }

        if (!query.exec(statement.remove("#"))) {
            qCritical() << "Failed to create table. SQL Statement:" << query.lastQuery() << "Error:" << query.lastError();
            return false;
        }
    }

    return true;
}
