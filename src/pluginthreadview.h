#ifndef PLUGINTHREADVIEW_H
#define PLUGINTHREADVIEW_H

#include <QObject>
#include <QDBusContext>
#include <QScopedPointer>
#include <QVariantMap>

namespace History {

class PluginThreadViewPrivate;

class PluginThreadView : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PluginThreadView)
public:
    explicit PluginThreadView(QObject *parent = 0);
    virtual ~PluginThreadView();

    // DBus exposed methods
    Q_NOREPLY void Destroy();
    virtual QList<QVariantMap> NextPage() = 0;
    virtual bool IsValid();

    // other methods
    QString objectPath() const;

Q_SIGNALS:
    void Invalidated();

private:
    QScopedPointer<PluginThreadViewPrivate> d_ptr;
};

}

#endif // PLUGINTHREADVIEW_H
