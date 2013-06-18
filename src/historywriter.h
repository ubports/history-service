#ifndef HISTORYWRITER_H
#define HISTORYWRITER_H

#include <TextItem>
#include <VoiceItem>

class HistoryWriter : public QObject
{
    Q_OBJECT
public:
    virtual ~HistoryWriter() {}

    virtual bool writeTextItem(const TextItem &item) = 0;
    virtual bool writeVoiceItem(const VoiceItem &item) = 0;

    // TODO: check if there is the need to write MMS entries
};

#endif
