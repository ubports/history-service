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
#include "voiceevent.h"

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
        qDebug() << "Usage: makevoiceevents accountId";
        return 1;
    }

    //TODO let the user pass numbers as args so they can corrspond to
    //the user's contacts

    QString number0 = "1234567890";
    QString number1 = "0987654321";
    QString number2 = "3216540987";
    QString number;
    int idx = 0;
    int control = 0;
    const QString accountId = args.at(1);
    QString sender;
    QDateTime datetime = QDateTime::currentDateTime();

    while (idx < 50) {
        QList<QString> participants_;
        idx ++;
        bool missed;
        if (control == 0) {
            qDebug() << "=== control is 0";
            number = number0;
            control++;
            missed = false;
            sender = "self";
        } else if (control == 1) {
            qDebug() << "=== control is 1";
            number = number1;
            control++;
            missed = true;
            sender = number;
        } else {
            qDebug() << "=== control is else";
            control = 0;
            number = number2;
            datetime = datetime.addDays(-1);
            missed = false;
            sender = number;
        }

        Thread thread = Manager::instance()->threadForParticipants(accountId,
                                                                   EventTypeVoice,
                                                                   QStringList() << number,
                                                                   MatchCaseSensitive,
                                                                   true);
        const QString eventId = QString::number(QDateTime::currentMSecsSinceEpoch());
        bool newEvent = true;
        const QTime duration = QTime(0,10,20);

        VoiceEvent ev(accountId, thread.threadId(), eventId, sender, datetime, newEvent, missed, duration, number, QStringList() << number);
        QList<Event> events;
        events.append(ev);

        Manager::instance()->writeEvents(events);
    }
}
