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

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include <QVariant>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include "telepathyhelper.h"
#include "mockcontroller.h"
#include "handler.h"
#include "approver.h"

#include "manager.h"
#include "thread.h"
#include "textevent.h"
#include "voiceevent.h"

Q_DECLARE_METATYPE(Tp::CallChannelPtr)
Q_DECLARE_METATYPE(Tp::TextChannelPtr)
Q_DECLARE_METATYPE(QList<Tp::ContactPtr>)
Q_DECLARE_METATYPE(History::Threads)
Q_DECLARE_METATYPE(History::Events)

class DaemonTest : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void contactsReceived(QList<Tp::ContactPtr> contacts);

private Q_SLOTS:
    void initTestCase();
    void testMessageReceived();
    void testMessageSent();
    void testMissedCall();
    void testOutgoingCall();

    // helper slots
    void onPendingContactsFinished(Tp::PendingOperation*);

private:
    Approver *mApprover;
    Handler *mHandler;
};

void DaemonTest::initTestCase()
{
    qRegisterMetaType<Tp::Presence>();
    qRegisterMetaType<Tp::CallChannelPtr>();
    qRegisterMetaType<Tp::TextChannelPtr>();
    qRegisterMetaType<Tp::PendingOperation*>();
    qRegisterMetaType<QList<Tp::ContactPtr> >();
    qRegisterMetaType<History::Threads>();
    qRegisterMetaType<History::Events>();
    TelepathyHelper::instance();

    QSignalSpy spy(TelepathyHelper::instance(), SIGNAL(accountReady()));
    QTRY_COMPARE(spy.count(), 1);
    QTRY_VERIFY(TelepathyHelper::instance()->connected());

    mHandler = new Handler(this);
    TelepathyHelper::instance()->registerClient(mHandler, "HistoryTestHandler");
    QTRY_VERIFY(mHandler->isRegistered());

    // register the approver
    mApprover = new Approver(this);
    TelepathyHelper::instance()->registerClient(mApprover, "HistoryTestApprover");
    // Tp-qt does not set registered status to approvers
    QTRY_VERIFY(QDBusConnection::sessionBus().interface()->isServiceRegistered(TELEPHONY_SERVICE_APPROVER));

    // we need to wait in order to give telepathy time to notify about the approver and handler
    QTest::qWait(3000); 
}

void DaemonTest::testMessageReceived()
{
    QSignalSpy threadsAddedSpy(History::Manager::instance(), SIGNAL(threadsAdded(History::Threads)));
    QSignalSpy threadsModifiedSpy(History::Manager::instance(), SIGNAL(threadsModified(History::Threads)));
    QSignalSpy eventsAddedSpy(History::Manager::instance(), SIGNAL(eventsAdded(History::Events)));

    QVariantMap properties;
    QString sender = "123456789";
    QString message = "Test message";
    QDateTime sentTime = QDateTime::currentDateTime();
    properties["Sender"] = sender;
    properties["SentTime"] = sentTime.toString(Qt::ISODate);

    QSignalSpy handlerSpy(mHandler, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    MockController::instance()->placeIncomingMessage(message, properties);
    QTRY_COMPARE(threadsAddedSpy.count(), 1);
    History::Threads threads = threadsAddedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread thread = threads.first();
    QCOMPARE(thread.participants().count(), 1);
    QCOMPARE(thread.participants().first(), sender);

    QTRY_COMPARE(threadsModifiedSpy.count(), 1);
    threads = threadsModifiedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread modifiedThread = threads.first();
    QVERIFY(modifiedThread == thread);

    QTRY_COMPARE(eventsAddedSpy.count(), 1);
    History::Events events = eventsAddedSpy.first().first().value<History::Events>();
    QCOMPARE(events.count(), 1);
    History::TextEvent event = events.first();
    QCOMPARE(event.senderId(), sender);
    QCOMPARE(event.threadId(), modifiedThread.threadId());
    QVERIFY(modifiedThread.lastEvent() == event);
    QCOMPARE(event.timestamp().toString(Qt::ISODate), sentTime.toString(Qt::ISODate));
    QCOMPARE(event.message(), message);

    QTRY_COMPARE(handlerSpy.count(), 1);
    Tp::TextChannelPtr channel = handlerSpy.first().first().value<Tp::TextChannelPtr>();
    QVERIFY(channel);
    channel->requestClose();
}

void DaemonTest::testMessageSent()
{
    // Request the contact to start chatting to
    Tp::AccountPtr account = TelepathyHelper::instance()->account();
    QSignalSpy spy(this, SIGNAL(contactsReceived(QList<Tp::ContactPtr>)));

    QString recipient = "987654321";

    connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << recipient),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPendingContactsFinished(Tp::PendingOperation*)));

    QTRY_COMPARE(spy.count(), 1);

    QList<Tp::ContactPtr> contacts = spy.first().first().value<QList<Tp::ContactPtr> >();
    QCOMPARE(contacts.count(), 1);
    QCOMPARE(contacts.first()->id(), recipient);

    QSignalSpy spyTextChannel(mHandler, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    Q_FOREACH(Tp::ContactPtr contact, contacts) {
        account->ensureTextChat(contact, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".HistoryTestHandler");
    }
    QTRY_COMPARE(spyTextChannel.count(), 1);

    Tp::TextChannelPtr channel = spyTextChannel.first().first().value<Tp::TextChannelPtr>();
    QVERIFY(channel);

    QSignalSpy threadsAddedSpy(History::Manager::instance(), SIGNAL(threadsAdded(History::Threads)));
    QSignalSpy threadsModifiedSpy(History::Manager::instance(), SIGNAL(threadsModified(History::Threads)));
    QSignalSpy eventsAddedSpy(History::Manager::instance(), SIGNAL(eventsAdded(History::Events)));

    QString messageText = "Hello, big world!";
    Tp::PendingSendMessage *message = channel->send(messageText);

    QTRY_COMPARE(threadsAddedSpy.count(), 1);
    History::Threads threads = threadsAddedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread thread = threads.first();
    QCOMPARE(thread.participants().count(), 1);
    QCOMPARE(thread.participants().first(), recipient);

    QTRY_COMPARE(threadsModifiedSpy.count(), 1);
    threads = threadsModifiedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread modifiedThread = threads.first();
    QVERIFY(modifiedThread == thread);

    QTRY_COMPARE(eventsAddedSpy.count(), 1);
    History::Events events = eventsAddedSpy.first().first().value<History::Events>();
    QCOMPARE(events.count(), 1);
    History::TextEvent event = events.first();
    QCOMPARE(event.senderId(), QString("self"));
    QCOMPARE(event.threadId(), modifiedThread.threadId());
    QVERIFY(modifiedThread.lastEvent() == event);
    QCOMPARE(event.message(), messageText);

    channel->requestClose();
}

void DaemonTest::testMissedCall()
{
    QSignalSpy newCallSpy(mApprover, SIGNAL(newCall()));

    // create an incoming call
    QString callerId = "33333333";
    QVariantMap properties;
    properties["Caller"] = callerId;
    properties["State"] = "incoming";
    MockController::instance()->placeCall(properties);
    QTRY_COMPARE(newCallSpy.count(), 1);

    QSignalSpy threadsAddedSpy(History::Manager::instance(), SIGNAL(threadsAdded(History::Threads)));
    QSignalSpy threadsModifiedSpy(History::Manager::instance(), SIGNAL(threadsModified(History::Threads)));
    QSignalSpy eventsAddedSpy(History::Manager::instance(), SIGNAL(eventsAdded(History::Events)));

    // now hangup the call and check that the event was added to the database
    MockController::instance()->hangupCall(callerId);

    QTRY_COMPARE(threadsAddedSpy.count(), 1);
    History::Threads threads = threadsAddedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread thread = threads.first();
    QCOMPARE(thread.participants().count(), 1);
    QCOMPARE(thread.participants().first(), callerId);

    QTRY_COMPARE(threadsModifiedSpy.count(), 1);
    threads = threadsModifiedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread modifiedThread = threads.first();
    QVERIFY(modifiedThread == thread);

    QTRY_COMPARE(eventsAddedSpy.count(), 1);
    History::Events events = eventsAddedSpy.first().first().value<History::Events>();
    QCOMPARE(events.count(), 1);
    History::VoiceEvent event = events.first();
    QCOMPARE(event.senderId(), callerId);
    QCOMPARE(event.threadId(), modifiedThread.threadId());
    QVERIFY(modifiedThread.lastEvent() == event);
    QCOMPARE(event.missed(), true);
}

void DaemonTest::testOutgoingCall()
{
    // Request the contact to start chatting to
    Tp::AccountPtr account = TelepathyHelper::instance()->account();
    QString phoneNumber = "44444444";
    QSignalSpy spy(this, SIGNAL(contactsReceived(QList<Tp::ContactPtr>)));

    connect(account->connection()->contactManager()->contactsForIdentifiers(QStringList() << phoneNumber),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPendingContactsFinished(Tp::PendingOperation*)));
    QTRY_COMPARE(spy.count(), 1);

    QList<Tp::ContactPtr> contacts = spy.first().first().value<QList<Tp::ContactPtr> >();
    QCOMPARE(contacts.count(), 1);
    QCOMPARE(contacts.first()->id(), phoneNumber);

    QSignalSpy spyCallChannel(mHandler, SIGNAL(callChannelAvailable(Tp::CallChannelPtr)));

    Q_FOREACH(Tp::ContactPtr contact, contacts) {
        account->ensureAudioCall(contact, "audio", QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".HistoryTestHandler");
    }
    QTRY_COMPARE(spyCallChannel.count(), 1);

    Tp::CallChannelPtr channel = spyCallChannel.first().first().value<Tp::CallChannelPtr>();
    QVERIFY(channel);

    MockController::instance()->setCallState(phoneNumber, "alerting");
    QTRY_COMPARE(channel->callState(), Tp::CallStateInitialised);

    MockController::instance()->setCallState(phoneNumber, "active");
    QTRY_COMPARE(channel->callState(), Tp::CallStateActive);

    QSignalSpy threadsAddedSpy(History::Manager::instance(), SIGNAL(threadsAdded(History::Threads)));
    QSignalSpy threadsModifiedSpy(History::Manager::instance(), SIGNAL(threadsModified(History::Threads)));
    QSignalSpy eventsAddedSpy(History::Manager::instance(), SIGNAL(eventsAdded(History::Events)));

    MockController::instance()->setCallState(phoneNumber, "disconnected");
    QTRY_COMPARE(channel->callState(), Tp::CallStateEnded);

    QTRY_COMPARE(threadsAddedSpy.count(), 1);
    History::Threads threads = threadsAddedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread thread = threads.first();
    QCOMPARE(thread.participants().count(), 1);
    QCOMPARE(thread.participants().first(), phoneNumber);

    QTRY_COMPARE(threadsModifiedSpy.count(), 1);
    threads = threadsModifiedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread modifiedThread = threads.first();
    QVERIFY(modifiedThread == thread);

    QTRY_COMPARE(eventsAddedSpy.count(), 1);
    History::Events events = eventsAddedSpy.first().first().value<History::Events>();
    QCOMPARE(events.count(), 1);
    History::VoiceEvent event = events.first();
    QCOMPARE(event.senderId(), QString("self"));
    QCOMPARE(event.threadId(), modifiedThread.threadId());
    QVERIFY(modifiedThread.lastEvent() == event);
    QCOMPARE(event.missed(), false);
    QVERIFY(event.duration().isValid());
}

void DaemonTest::onPendingContactsFinished(Tp::PendingOperation *op)
{
    Tp::PendingContacts *pc = qobject_cast<Tp::PendingContacts*>(op);
    if (!pc) {
        return;
    }

    Q_EMIT contactsReceived(pc->contacts());
}

QTEST_MAIN(DaemonTest)
#include "DaemonTest.moc"
