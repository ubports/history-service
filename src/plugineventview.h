#ifndef PLUGINEVENTVIEW_H
#define PLUGINEVENTVIEW_H

#include <QObject>
#include <QDBusContext>
#include <QScopedPointer>
#include <QVariantMap>

namespace History {

class PluginEventViewPrivate;

class PluginEventView : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(PluginEventView)
public:
    explicit PluginEventView(QObject *parent = 0);
    virtual ~PluginEventView();

    // DBus exposed methods
    Q_NOREPLY void Destroy();
    virtual QList<QVariantMap> NextPage() = 0;
    virtual bool IsValid();

    // other methods
    QString objectPath() const;

Q_SIGNALS:
    void Invalidated();

private:
    QScopedPointer<PluginEventViewPrivate> d_ptr;
};

}

#endif // PLUGINEVENTVIEW_H
