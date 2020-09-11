/**
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 */

#include <QDebug>

#include <TelepathyQt/Constants>
#include <TelepathyQt/DBusObject>

#include "speakeriface.h"

// Chan.I.Speaker
BaseChannelSpeakerInterface::Adaptee::Adaptee(BaseChannelSpeakerInterface *interface)
    : QObject(interface),
      mInterface(interface)
{
}

struct TP_QT_NO_EXPORT BaseChannelSpeakerInterface::Private {
    Private(BaseChannelSpeakerInterface *parent)
        : speakerMode(false),
          adaptee(new BaseChannelSpeakerInterface::Adaptee(parent)) {
    }
    bool speakerMode;
    turnOnSpeakerCallback turnOnSpeakerCB;
    BaseChannelSpeakerInterface::Adaptee *adaptee;
};

BaseChannelSpeakerInterface::Adaptee::~Adaptee()
{
}

void BaseChannelSpeakerInterface::Adaptee::turnOnSpeaker(bool active, const ChannelInterfaceSpeakerAdaptor::turnOnSpeakerContextPtr &context)
{
    if (!mInterface->mPriv->turnOnSpeakerCB.isValid()) {
        context->setFinishedWithError(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented"));
        return;
    }
    Tp::DBusError error;
    mInterface->mPriv->turnOnSpeakerCB(active, &error);
    if (error.isValid()) {
        context->setFinishedWithError(error.name(), error.message());
        return;
    }
    context->setFinished();
}

BaseChannelSpeakerInterface::BaseChannelSpeakerInterface()
    : AbstractChannelInterface(TP_QT_IFACE_CHANNEL_SPEAKER),
      mPriv(new Private(this))
{
}

BaseChannelSpeakerInterface::~BaseChannelSpeakerInterface()
{
    delete mPriv;
}

bool BaseChannelSpeakerInterface::speakerMode() const
{
    return mPriv->speakerMode;
}

void BaseChannelSpeakerInterface::setTurnOnSpeakerCallback(const turnOnSpeakerCallback &cb)
{
    mPriv->turnOnSpeakerCB = cb;
}

void BaseChannelSpeakerInterface::setSpeakerMode(bool active)
{
    mPriv->speakerMode = active;
    Q_EMIT mPriv->adaptee->speakerChanged(active);
}

QVariantMap BaseChannelSpeakerInterface::immutableProperties() const
{
    QVariantMap map;
    return map;
}

void BaseChannelSpeakerInterface::createAdaptor()
{
    (void) new ChannelInterfaceSpeakerAdaptor(dbusObject()->dbusConnection(),
            mPriv->adaptee, dbusObject());
}


ChannelInterfaceSpeakerAdaptor::ChannelInterfaceSpeakerAdaptor(const QDBusConnection& bus, QObject* adaptee, QObject* parent)
    : Tp::AbstractAdaptor(bus, adaptee, parent)
{
    connect(adaptee, SIGNAL(speakerChanged(bool)), SIGNAL(SpeakerChanged(bool)));
}

ChannelInterfaceSpeakerAdaptor::~ChannelInterfaceSpeakerAdaptor()
{
}

void ChannelInterfaceSpeakerAdaptor::turnOnSpeaker(bool active, const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("turnOnSpeaker(bool,ChannelInterfaceSpeakerAdaptor::turnOnSpeakerContextPtr)")) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return;
    }

    turnOnSpeakerContextPtr ctx = turnOnSpeakerContextPtr(
            new Tp::MethodInvocationContext< bool >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "turnOnSpeaker",
        Q_ARG(bool, active),
        Q_ARG(ChannelInterfaceSpeakerAdaptor::turnOnSpeakerContextPtr, ctx));
    return;
}

bool ChannelInterfaceSpeakerAdaptor::SpeakerMode() const
{
    return qvariant_cast< bool >(adaptee()->property("speakerMode"));
}
