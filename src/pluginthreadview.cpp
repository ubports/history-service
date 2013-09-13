#include "pluginthreadview.h"
#include "pluginthreadview_p.h"
#include "pluginthreadviewadaptor.h"
#include "types.h"
#include <QDBusConnection>
#include <QDebug>

Q_DECLARE_METATYPE(QList< QVariantMap >)

namespace History {

PluginThreadViewPrivate::PluginThreadViewPrivate()
    : adaptor(0)
{
}

PluginThreadView::PluginThreadView(QObject *parent) :
    QObject(parent), d_ptr(new PluginThreadViewPrivate())
{
    Q_D(PluginThreadView);
    qDBusRegisterMetaType<QList<QVariantMap> >();

    d->adaptor = new ThreadViewAdaptor(this);

    QString id = QString("threadview%1%2").arg(QString::number((qulonglong)this), QDateTime::currentDateTimeUtc().toString("yyyyMMddhhmmsszzz"));
    d->objectPath = QString("%1/%2").arg(History::DBusObjectPath, id);
    qDebug() << "Registering: " << QDBusConnection::sessionBus().registerObject(d->objectPath, this);
    qDebug() << "Object path:" << d->objectPath;
}

PluginThreadView::~PluginThreadView()
{
    Q_D(PluginThreadView);
    QDBusConnection::sessionBus().unregisterObject(d->objectPath);
}

void PluginThreadView::Destroy()
{
    Q_D(PluginThreadView);
    deleteLater();
}

bool PluginThreadView::IsValid()
{
    qDebug() << __PRETTY_FUNCTION__;
    return true;
}

QString PluginThreadView::objectPath() const
{
    Q_D(const PluginThreadView);
    return d->objectPath;
}

}
