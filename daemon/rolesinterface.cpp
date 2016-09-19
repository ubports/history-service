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
