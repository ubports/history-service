#ifndef HISTORYPLUGIN_H
#define HISTORYPLUGIN_H

class HistorySaver;
class HistoryReader;

class HistoryPlugin
{
public:
    virtual ~HistoryPlugin() {}
    virtual HistorySaver *saver() const = 0;
    virtual HistoryReader *reader() const = 0;
};

#endif
