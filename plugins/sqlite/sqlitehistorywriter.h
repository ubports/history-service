#ifndef SQLITEHISTORYWRITER_H
#define SQLITEHISTORYWRITER_H

#include <QStringList>
#include "writer.h"
#include "types.h"

class SQLiteHistoryWriter : public History::Writer
{
    Q_OBJECT
public:
    explicit SQLiteHistoryWriter(QObject *parent = 0);

    History::ThreadPtr threadForParticipants(const QString &accountId, History::EventType type, const QStringList &participants);
    bool writeTextEvent(const History::TextEventPtr &event);
    bool writeVoiceEvent(const History::VoiceEventPtr &event);
};

#endif // SQLITEHISTORYWRITER_H
