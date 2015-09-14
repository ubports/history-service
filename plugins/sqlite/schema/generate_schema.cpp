/*
 * Copyright (C) 2015 Canonical, Ltd.
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
