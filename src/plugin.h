#ifndef HISTORY_PLUGIN_H
#define HISTORY_PLUGIN_H

#include <QtPlugin>
#include "types.h"

namespace History
{

class Writer;
class Reader;

class Plugin
{
public:
    virtual ~Plugin() {}
    virtual WriterPtr writer() const = 0;
    virtual ReaderPtr reader() const = 0;
};

}

Q_DECLARE_INTERFACE(History::Plugin, "com.canonical.historyservice.Plugin")

#endif // HISTORY_PLUGIN_H
