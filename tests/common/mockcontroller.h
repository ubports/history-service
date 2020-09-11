/**
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

#ifndef MOCKCONTROLLER_H
#define MOCKCONTROLLER_H

#include <QObject>
#include <QDBusInterface>

class MockController : public QObject
{
    Q_OBJECT
public:
    explicit MockController(const QString &protocol, QObject *parent = 0);

Q_SIGNALS:
    void messageRead(const QString &messageId);
    void messageSent(const QString &message, const QVariantMap &properties);
    void callReceived(const QString &callerId);
    void callEnded(const QString &callerId);
    void callStateChanged(const QString &callerId, const QString &objectPath, const QString &state);
    void conferenceCreated(const QString &objectPath);
    void channelMerged(const QString &objectPath);
    void channelSplitted(const QString &objectPath);
    void disconnected();
    void connectionDestroyed();

public Q_SLOTS:
    void placeIncomingMessage(const QString &message, const QVariantMap &properties);
    void placeDeliveryReport(const QStringList &recipients, const QString &messageId, const QString &status);
    QString placeCall(const QVariantMap &properties);
    void hangupCall(const QString &callerId);
    void setCallState(const QString &phoneNumber, const QString &state);
    void setOnline(bool online);
    void setPresence(const QString &status, const QString &statusMessage);

    // voicemail stuff
    void setVoicemailNumber(const QString &number);
    void setVoicemailIndicator(bool active);
    void setVoicemailCount(int count);

    // emergency numbers stuff
    void setEmergencyNumbers(const QStringList &numbers);

    // USSD stuff
    QString serial();

private:
    QString mProtocol;
    QString mMockObject;
    QDBusInterface mMockInterface;
};

#endif // MOCKCONTROLLER_H
