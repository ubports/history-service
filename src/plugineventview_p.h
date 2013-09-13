#ifndef PLUGINEVENTVIEW_P_H
#define PLUGINEVENTVIEW_P_H

#include <QScopedPointer>

class EventViewAdaptor;

namespace History {

class PluginEventViewPrivate
{
public:
    PluginEventViewPrivate();

    EventViewAdaptor *adaptor;
    QString objectPath;
};

}

#endif // PLUGINEVENTVIEW_P_H
