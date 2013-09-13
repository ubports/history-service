#ifndef PLUGINTHREADVIEW_H
#define PLUGINTHREADVIEW_H

#include <QObject>
#include <QDBusContext>
#include <QVariantMap>

class PluginThreadView : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit PluginThreadView(QObject *parent = 0);
    virtual ~PluginThreadView();

    // DBus exposed methods
    Q_NOREPLY void Destroy();
    virtual QList<QVariantMap> NextPage() = 0;
    virtual bool IsValid() { return true; }

Q_SIGNALS:
    void Invalidated();
    
};

#endif // PLUGINTHREADVIEW_H
