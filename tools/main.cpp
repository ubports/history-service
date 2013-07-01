#include <HistoryManager>
#include <HistoryThread>
#include <TextItem>
#include <VoiceItem>
#include <QCoreApplication>
#include <QDebug>

void printItem(const HistoryItemPtr &item)
{
    QString extraInfo;
    TextItemPtr textItem;
    VoiceItemPtr voiceItem;

    switch (item->type()) {
    case HistoryItem::TextItem:
        textItem = item.staticCast<TextItem>();
        extraInfo = QString(" message: %1").arg(textItem->message());
        break;
    case HistoryItem::VoiceItem:
        voiceItem = item.staticCast<VoiceItem>();
        extraInfo = QString(" missed: %1 duration: %2").arg(voiceItem->missed() ? "yes" : "no", voiceItem->duration().toString());
        break;
    }

    qDebug() << QString("    * Item: accountId: %1 threadId: %2 itemId: %3 sender: %4 timestamp: %5 newItem: %6")
                .arg(item->accountId(), item->threadId(), item->itemId(), item->sender(), item->timestamp().toString(),
                     item->newItem() ? "yes" : "no");
    qDebug() << QString("      %1").arg(extraInfo);
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
