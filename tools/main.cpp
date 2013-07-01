#include <HistoryManager>
#include <HistoryThread>
#include <QCoreApplication>
#include <QDebug>

void printItem(const HistoryItemPtr &item)
{

}

void printThread(const HistoryThreadPtr &thread)
{
    QString type = "Unknown";
    switch (thread->type()) {
    case HistoryItem::TextItem:
        type = "Text";
        break;
    case HistoryItem::VoiceItem:
        type = "Voice";
        break;
    }

    qDebug() << QString("%1 thread - accountId: %2 threadId: %3 count: %4 unreadCount: %5").arg(type,
                                                                                                thread->accountId(),
                                                                                                thread->threadId(),
                                                                                                QString::number(thread->count()),
                                                                                                QString::number(thread->unreadCount()));
    qDebug() << QString("    Participants: %1").arg(thread->participants().join(", "));
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    HistoryManager *manager = HistoryManager::instance();

    // voice items
    Q_FOREACH(const HistoryThreadPtr &thread, manager->queryThreads(HistoryItem::VoiceItem)) {
        printThread(thread);
    }

    // text items
    Q_FOREACH(const HistoryThreadPtr &thread, manager->queryThreads(HistoryItem::TextItem)) {
        printThread(thread);
    }

    // for now print all items, it is not possible to filter for one given thread and account id yet
    Q_FOREACH(const HistoryItemPtr &item, manager->queryItems(HistoryItem::VoiceItem)) {
        printItem(item);
    }

    Q_FOREACH(const HistoryItemPtr &item, manager->queryItems(HistoryItem::TextItem)) {
        printItem(item);
    }
    return app.exec();
}
