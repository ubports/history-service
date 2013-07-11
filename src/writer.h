#ifndef HISTORYWRITER_H
#define HISTORYWRITER_H

#include "types.h"

namespace History
{

class Writer : public QObject
{
    Q_OBJECT
public:
    explicit Writer(QObject *parent = 0) : QObject(parent) {}
    virtual ~Writer() {}

    virtual ThreadPtr threadForParticipants(const QString &accountId, EventType type, const QStringList &participants) = 0;
    virtual bool writeTextEvent(const TextEventPtr &event) = 0;
    virtual bool writeVoiceEvent(const VoiceEventPtr &event) = 0;

    virtual bool beginBatchOperation() {}
    virtual bool endBatchOperation() {}

    // TODO: check if there is the need to write MMS entries
};

}

#endif
