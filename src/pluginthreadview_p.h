#ifndef PLUGINTHREADVIEW_P_H
#define PLUGINTHREADVIEW_P_H

#include <QScopedPointer>

class ThreadViewAdaptor;

namespace History {

class PluginThreadViewPrivate
{
public:
    PluginThreadViewPrivate();

    ThreadViewAdaptor *adaptor;
    QString objectPath;
};

}

#endif // PLUGINTHREADVIEW_P_H
