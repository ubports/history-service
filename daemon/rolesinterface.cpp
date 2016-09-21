#include <daemon/rolesinterface.h>

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), QDBusConnection::sessionBus(), parent)
{
}

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(const QDBusConnection& connection, const QString& busName, const QString& objectPath, QObject *parent)
    : Tp::AbstractInterface(busName, objectPath, staticInterfaceName(), connection, parent)
{
}

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(Tp::DBusProxy *proxy)
    : Tp::AbstractInterface(proxy, staticInterfaceName())
{
}

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(const Tp::Client::ChannelInterface& mainInterface)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), mainInterface.parent())
{
}

ChannelInterfaceRolesInterface::ChannelInterfaceRolesInterface(const Tp::Client::ChannelInterface& mainInterface, QObject *parent)
    : Tp::AbstractInterface(mainInterface.service(), mainInterface.path(), staticInterfaceName(), mainInterface.connection(), parent)
{
}

void ChannelInterfaceRolesInterface::invalidate(Tp::DBusProxy *proxy,
        const QString &error, const QString &message)
{
    Tp::AbstractInterface::invalidate(proxy, error, message);
}

HandleRolesMap ChannelInterfaceRolesInterface::getRoles() const
{
    QDBusMessage msg = QDBusMessage::createMethodCall(service(), path(),
            TP_QT_IFACE_PROPERTIES, QLatin1String("Get"));
    msg << interface() << QLatin1String("Roles");
    QDBusMessage result = connection().call(msg);
    return qdbus_cast<HandleRolesMap>(result.arguments().at(0).value<QDBusVariant>().variant());
}

bool ChannelInterfaceRolesInterface::getCanUpdateRoles() const
{
    QDBusMessage msg = QDBusMessage::createMethodCall(service(), path(),
            TP_QT_IFACE_PROPERTIES, QLatin1String("Get"));
    msg << interface() << QLatin1String("CanUpdateRoles");
    QDBusMessage result = connection().call(msg);
    return qdbus_cast<bool>(result.arguments().at(0).value<QDBusVariant>().variant());
}
