#include "manager.h"
#include "eventview.h"
#include "filter.h"
#include "intersectionfilter.h"
#include "thread.h"
#include "threadview.h"
#include "textevent.h"
#include "voiceevent.h"
#include <QCoreApplication>
#include <QDebug>

void printEvent(const History::EventPtr &event)
{
    QString extraInfo;
    History::TextEventPtr textEvent;
    History::VoiceEventPtr voiceEvent;

    switch (event->type()) {
    case History::EventTypeText:
        textEvent = event.staticCast<History::TextEvent>();
        extraInfo = QString(" message: %1").arg(textEvent->message());
        break;
    case History::EventTypeVoice:
        voiceEvent = event.staticCast<History::VoiceEvent>();
        extraInfo = QString(" missed: %1 duration: %2").arg(voiceEvent->missed() ? "yes" : "no", voiceEvent->duration().toString());
        break;
    }

    qDebug() << qPrintable(QString("    * Item: accountId: %1 threadId: %2 eventId: %3 sender: %4 timestamp: %5 newEvent: %6")
                .arg(event->accountId(), event->threadId(), event->eventId(), event->sender(), event->timestamp().toString(),
                     event->newEvent() ? "yes" : "no"));
    qDebug() << qPrintable(QString("      %1").arg(extraInfo));
}

void printThread(const History::ThreadPtr &thread)
{
    QString type = "Unknown";
    switch (thread->type()) {
    case History::EventTypeText:
        type = "Text";
        break;
    case History::EventTypeVoice:
        type = "Voice";
        break;
    }

    qDebug() << qPrintable(QString("%1 thread - accountId: %2 threadId: %3 count: %4 unreadCount: %5").arg(type,
                                                                                                thread->accountId(),
                                                                                                thread->threadId(),
                                                                                                QString::number(thread->count()),
                                                                                                QString::number(thread->unreadCount())));
    qDebug() << qPrintable(QString("    Participants: %1").arg(thread->participants().join(", ")));

    if (!thread->lastEvent().isNull()) {
        qDebug() << "    Last event:";
        printEvent(thread->lastEvent());
    }
    qDebug() << "    All events:";
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    History::Manager *manager = History::Manager::instance();

    QList<History::EventType> eventTypes;
    eventTypes << History::EventTypeText << History::EventTypeVoice;

    Q_FOREACH(History::EventType type, eventTypes) {
        History::ThreadViewPtr view = manager->queryThreads(type);
        QList<History::ThreadPtr> threads = view->nextPage();

        while (!threads.isEmpty()) {
            Q_FOREACH(const History::ThreadPtr &thread, threads) {
                printThread(thread);

                // now print the items for this thread
                History::IntersectionFilterPtr filter(new History::IntersectionFilter());
                filter->append(History::FilterPtr(new History::Filter("threadId", thread->threadId())));
                filter->append(History::FilterPtr(new History::Filter("accountId", thread->accountId())));
                History::EventViewPtr eventView = manager->queryEvents(type, History::SortPtr(), filter);
                QList<History::EventPtr> events = eventView->nextPage();
                while (!events.isEmpty()) {
                    Q_FOREACH(const History::EventPtr &event, events) {
                        printEvent(event);
                    }
                    events = eventView->nextPage();
                }
            }
            threads = view->nextPage();
        }
    }
}
