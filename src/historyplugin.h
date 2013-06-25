#ifndef HISTORYPLUGIN_H
#define HISTORYPLUGIN_H

#include <QtPlugin>
#include <Types>

class HistoryWriter;
class HistoryReader;

class HistoryPlugin
{
public:
    virtual ~HistoryPlugin() {}
    virtual HistoryWriterPtr writer() const = 0;
    virtual HistoryReaderPtr reader() const = 0;
};

Q_DECLARE_INTERFACE(HistoryPlugin, "com.canonical.libhistory.HistoryPlugin")

#endif
