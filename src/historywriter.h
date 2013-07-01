#ifndef HISTORYWRITER_H
#define HISTORYWRITER_H

#include <TextItem>
#include <VoiceItem>
#include <Types>

class HistoryWriter : public QObject
{
    Q_OBJECT
public:
    explicit HistoryWriter(QObject *parent = 0) : QObject(parent) {}
    virtual ~HistoryWriter() {}

    virtual HistoryThreadPtr threadForParticipants(const QString &accountId, HistoryItem::ItemType type, const QStringList &participants) = 0;
    virtual bool writeTextItem(const TextItem &item) = 0;
    virtual bool writeVoiceItem(const VoiceItem &item) = 0;

    // TODO: check if there is the need to write MMS entries
};

#endif
