#include "manager.h"
#include "types.h"
#include "voiceevent.h"
#include "textevent.h"

#include <QCoreApplication>
#include <QtXml>
#include <QtCore>
#include <QDateTime>
#include <QMap>
#include <QList>

using namespace History;

QList<QMap<QString, QString> >readMessages(QString path) {
    QFile file(path);

    QList<QMap<QString, QString> > messages;

    file.open(QFile::ReadOnly | QFile::Text);

    QDomDocument dom;
    QString error;

    int line, column;

    if (!dom.setContent(&file, &error, &line, &column)) {
        qDebug() << "Error:" << error << "in line " << line << "column" << column;
        return messages;
    }

    QDomNodeList nodes = dom.elementsByTagName("message");

    for (int i = 0; i < nodes.count(); i++)
    {
        QDomNode elm = nodes.at(i);

        if (elm.isElement())
        {
            QMap<QString, QString> map;

            QString time     = elm.toElement().attribute("date");
            QString senderid = elm.toElement().attribute("address");
            QString number   = elm.toElement().attribute("address");
            QString message  = elm.toElement().text();
            QString type     = elm.toElement().attribute("type");

            if (type == "2") {
                senderid = "self";
            }

            map["time"]     = time;
            map["senderid"] = senderid;
            map["number"]   = number;
            map["message"]  = message;

            // qDebug() << "Entry: " << map;
            messages.append(map);
        }
    }
    return messages;
}

QList<QMap<QString, QString> >readCalls(QString path) {
    QFile file(path);

    QList<QMap<QString, QString> > calls;

    file.open(QFile::ReadOnly | QFile::Text);

    QDomDocument dom;
    QString error;

    int line, column;

    if (!dom.setContent(&file, &error, &line, &column)) {
        qDebug() << "Error:" << error << "in line " << line << "column" << column;
        return calls;
    }

    QDomNodeList nodes = dom.elementsByTagName("call");

    for (int i = 0; i < nodes.count(); i++)
    {
        QDomNode elm = nodes.at(i);

        if (elm.isElement())
        {
            QMap<QString, QString> map;

            QString time     = elm.toElement().attribute("date");
            QString senderid = elm.toElement().attribute("number");
            QString number   = elm.toElement().attribute("number");
            QString duration = elm.toElement().attribute("duration");
            QString missed   = elm.toElement().attribute("type");

            if (elm.toElement().hasAttribute("formatted_number")) {
                senderid = "self";
            }

            map["time"]     = time;
            map["senderid"] = senderid;
            map["number"]   = number;
            map["duration"] = duration;
            map["missed"]   = missed;

            // qDebug() << "Entry: " << map;
            calls.append(map);
        }
    }
    return calls;
}

void writeCalls(QString accountId, QList<QMap<QString, QString> >calls) {
    for (auto call : calls) {
        Thread thread = Manager::instance()->threadForParticipants(
            accountId,
            EventTypeVoice,
            QStringList() << call["number"],
                MatchCaseSensitive,
                true);

        QDateTime datetime = QDateTime::fromMSecsSinceEpoch(
            call["time"].toLong());
        bool missed = false;

        if (call["missed"] == "3") {
            missed = true;
        }
        const QString eventId =
            QString("%1:%2").arg(call["number"], call["time"]);
        bool newEvent        = false;
        const QTime duration = QTime().addSecs(call["duration"].toInt());

        VoiceEvent ev(accountId,
                      thread.threadId(),
                      eventId,
                      call["senderid"],
                      datetime,
                      newEvent,
                      missed,
                      duration,
                      call["number"],
                      thread.participants());
        QList<Event> events;
        events.append(ev);
        Manager::instance()->writeEvents(events);
    }
}

void writeMessages(QString accountId, QList<QMap<QString, QString> >messages) {
    for (auto message : messages) {
        Thread thread = Manager::instance()->threadForParticipants(
            accountId,
            EventTypeText,
            QStringList() << message["number"],
                MatchCaseSensitive,
                true);

        QDateTime datetime = QDateTime::fromMSecsSinceEpoch(
            message["time"].toLong());
        const QString eventId =
            QString("%1:%2").arg(message["number"],
                                 datetime.toString(Qt::ISODate));
        const QTime duration = QTime().addSecs(message["duration"].toInt());

        TextEvent ev(accountId,
                     thread.threadId(),
                     eventId,
                     message["senderid"],
                     datetime,
                     false,
                     message["message"],
                     MessageTypeText,
                     MessageStatusRead,
                     datetime,
                     "");
        QList<Event> events;
        events.append(ev);
        Manager::instance()->writeEvents(events);
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();
    if (args.size() == 1) {
        qDebug() << "Usage: import-backup ofono/ofono/account1 calllogs_export.xml messages_export.xml";
        return 1;
    }
    else if (args.size() != 4) {
        qDebug() << "Usage: import-backup ofono/ofono/account1 calllogs_export.xml messages_export.xml";
        return 1;
    }

    const QString accountId = args.at(1);
    const QString call_export = args.at(2);
    const QString message_export = args.at(3);

    auto calls = readCalls(QString(call_export));

    qDebug() << "Extracted:" << calls.count() << "calls";
    qDebug() << "Writing calls into db ...";
    writeCalls(accountId, calls);

    auto messages =
        readMessages(QString(message_export));

    qDebug() << "Extracted:" << messages.count() << "messages";
    qDebug() << "Writing messages into db ...";
    writeMessages(accountId, messages);
    qDebug() << "Finished!";
    return 0;
}
