#include <QCoreApplication>
#include "sqlitedatabase.h"
#include <QDebug>
#include <QFile>

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // force using a memory database
    setenv("HISTORY_SQLITE_DBPATH", ":memory:", 1);


    QStringList schemaFiles = app.arguments();
    // take argv[0] out of the list
    schemaFiles.takeFirst();

    // and store the target file too
    QString targetFile = schemaFiles.takeLast();

    QStringList statements;
    Q_FOREACH(const QString &file, schemaFiles) {
        statements << SQLiteDatabase::instance()->parseSchemaFile(file);
    }

    if (SQLiteDatabase::instance()->runMultipleStatements(statements)) {
        QFile file(targetFile);
        if (!file.open(QIODevice::WriteOnly)) {
            return 1;
        }
        file.write(SQLiteDatabase::instance()->dumpSchema().toUtf8().data());
        file.close();
        return 0;
    }
    return 1;
}
