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

#include "historydaemon.h"
#include <QLockFile>
#include <QDir>

bool checkApplicationRunning()
{
    QString lockPath = qgetenv("HISTORY_LOCK_FILE");
    if (lockPath.isEmpty()) {
        lockPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        QDir dir(lockPath);
        if (!dir.exists("history-service") && !dir.mkpath("history-service")) {
            qCritical() << "Failed to create dir";
            // in case we fail to create the lock, better not even start the application
            return true;
        }
        dir.cd("history-service");
        lockPath = dir.absoluteFilePath("history-daemon.lock");
    }

    static QLockFile *lockFile = new QLockFile(lockPath);
    return !lockFile->tryLock();
}
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (checkApplicationRunning()) {
        return 1;
    }

    HistoryDaemon::instance();
    return app.exec();
}
