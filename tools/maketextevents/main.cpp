/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by Kyle Nitzsche <kyle.nitzsche@canonical.com>
 *
 */

#include "manager.h"
#include "types.h"
#include "textevent.h"
#include "texteventattachment.h"

#include <QStringList>
#include <QString>
#include <QDebug>
#include <QCoreApplication>

using namespace History;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();
    if (args.size() == 1) {
        qDebug() << "Usage: maketextevents accountId";
        return 1;
    }
    //TODO let the user pass numbers as args so they can corrspond to
    //the user's contacts

    QString number0 = "1234567890";
    QString number1 = "01234456789";
    QString number2 = "0987654321";
    QString number;
    int idx = 0;
    QDateTime datetime = QDateTime::currentDateTime();
    int control = 0;
    while (idx < 50) {
        idx ++;
        if (control == 0) {
            qDebug() << "=== control is 0";
            number = number0;
            control++;
        } else if (control == 1) {
            qDebug() << "=== control is 1";
            number = number1;
            control++;
        } else {
            qDebug() << "=== control is else";
            control = 0;
            number = number2;
            datetime = datetime.addDays(-1);
        }

        const QString accountId = args.at(1);
        Thread thread = Manager::instance()->threadForParticipants(accountId,
                                                                   EventTypeText,
                                                                   QStringList() << number,
                                                                   MatchCaseSensitive,
                                                                   true);
        const QString threadId = thread.threadId();
        const QString eventId = QString::number(QDateTime::currentMSecsSinceEpoch());
        const QString sender = number;

        TextEvent ev(accountId,
                     threadId,
                     eventId,
                     sender,
                     datetime,
                     true,
                     QString::fromStdString("the text message"),
                     MessageTypeText,
                     MessageStatusUnknown,
                     datetime,
                     QString::fromStdString("the subject"));
        QList<Event> events;
        events.append(ev);
        Manager::instance()->writeEvents(events);
    }
}
