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
 * Authors:
 *     Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *     Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#ifndef MOCKCONFERENCECALLCHANNEL_H
#define MOCKCONFERENCECALLCHANNEL_H

#include <QObject>

#include <TelepathyQt/Constants>
#include <TelepathyQt/BaseChannel>
#include <TelepathyQt/BaseCall>
#include <TelepathyQt/Types>

#include "connection.h"
#include "speakeriface.h"

class MockConnection;

class MockConferenceCallChannel : public QObject
{
    Q_OBJECT
public:
    MockConferenceCallChannel(MockConnection *conn, QList<QDBusObjectPath> callChannels, QObject *parent = 0);
    ~MockConferenceCallChannel();

    void onHangup(uint reason, const QString &detailedReason, const QString &message, Tp::DBusError* error);
    void onMuteStateChanged(const Tp::LocalMuteState &state, Tp::DBusError *error);
    void onHoldStateChanged(const Tp::LocalHoldState &state, const Tp::LocalHoldStateReason &reason, Tp::DBusError *error);
    void onDTMFStartTone(uchar event, Tp::DBusError *error);
    void onDTMFStopTone(Tp::DBusError *error);
    void onTurnOnSpeaker(bool active, Tp::DBusError *error);
    void onMerge(const QDBusObjectPath &channel, Tp::DBusError *error);
    Tp::BaseChannelPtr baseChannel();
    void setConferenceActive(bool active);

Q_SIGNALS:
    void channelMerged(const QString &objectPath);
    void initialized();

private Q_SLOTS:
    void onDtmfComplete(bool success);
    void sendNextDtmf();
    void init();

    void onOfonoMuteChanged(bool mute);
    void onChannelSplitted(const QDBusObjectPath &path);

private:
    bool mRequestedHangup;
    MockConnection *mConnection;
    bool mDtmfLock;
    QList<QDBusObjectPath> mCallChannels;
    QString mObjPath;
    QString mPreviousState;
    bool mIncoming;
    Tp::BaseChannelPtr mBaseChannel;
    Tp::BaseChannelHoldInterfacePtr mHoldIface;
    Tp::BaseChannelConferenceInterfacePtr mConferenceIface;
    Tp::BaseChannelMergeableConferenceInterfacePtr mMergeableIface;
    Tp::BaseCallMuteInterfacePtr mMuteIface;
    BaseChannelSpeakerInterfacePtr mSpeakerIface;
    Tp::BaseChannelCallTypePtr mCallChannel;
    Tp::BaseCallContentDTMFInterfacePtr mDTMFIface;
    QStringList mDtmfPendingStrings;
};

#endif // MOCKCONFERENCECALLCHANNEL_H
