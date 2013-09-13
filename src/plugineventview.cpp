#include "plugineventview.h"
#include "plugineventview_p.h"
#include "plugineventviewadaptor.h"
#include "types.h"
#include <QDBusConnection>
#include <QDebug>

Q_DECLARE_METATYPE(QList< QVariantMap >)

namespace History {

PluginEventViewPrivate::PluginEventViewPrivate()
    : adaptor(0)
{
}

PluginEventView::PluginEventView(QObject *parent) :
    QObject(parent), d_ptr(new PluginEventViewPrivate())
{
    Q_D(PluginEventView);
    qDBusRegisterMetaType<QList<QVariantMap> >();

    d->adaptor = new EventViewAdaptor(this);

    QString id = QString("eventview%1%2").arg(QString::number((qulonglong)this), QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmsszzz"));
    d->objectPath = QString("%1/%2").arg(History::DBusObjectPath, id);
    QDBusConnection::sessionBus().registerObject(d->objectPath, this);
}

PluginEventView::~PluginEventView()
{
    Q_D(PluginEventView);
    QDBusConnection::sessionBus().unregisterObject(d->objectPath);
}

void PluginEventView::Destroy()
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_D(PluginEventView);
    deleteLater();
}

bool PluginEventView::IsValid()
{
    return true;
}

QString PluginEventView::objectPath() const
{
    Q_D(const PluginEventView);
    return d->objectPath;
}

}
