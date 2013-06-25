#ifndef SQLITEHISTORYWRITER_H
#define SQLITEHISTORYWRITER_H

#include <QStringList>
#include <HistoryWriter>
#include <Types>

class SQLiteHistoryWriter : public HistoryWriter
{
    Q_OBJECT
public:
    explicit SQLiteHistoryWriter(QObject *parent = 0);

    HistoryThreadPtr threadForParticipants(const QString &accountId, HistoryItem::ItemType type, const QStringList &participants);
    bool writeTextItem(const TextItem &item);
    bool writeVoiceItem(const VoiceItem &item);
};

#endif // SQLITEHISTORYWRITER_H
