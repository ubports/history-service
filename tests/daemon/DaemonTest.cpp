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
#include <TelepathyQt/Types>
#include <TelepathyQt/Account>
#include <TelepathyQt/CallChannel>
#include <TelepathyQt/Contact>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include "telepathyhelper_p.h"
#include "telepathytest.h"
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
Q_DECLARE_METATYPE(History::MessageStatus)

class DaemonTest : public TelepathyTest
{
    Q_OBJECT

Q_SIGNALS:
    void contactsReceived(QList<Tp::ContactPtr> contacts);

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testMessageReceived();
    void testMessageSentNoEventId();
    void testMessageSent();
    void testMissedCall();
    void testOutgoingCall();
    void testDeliveryReport_data();
    void testDeliveryReport();

    // helper slots
    void onPendingContactsFinished(Tp::PendingOperation*);

private:
    Approver *mApprover;
    Handler *mHandler;
    MockController *mMockController;
    Tp::AccountPtr mAccount;
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
    qRegisterMetaType<History::MessageStatus>();

    initialize();

    // register the handler
    mHandler = new Handler(this);
    History::TelepathyHelper::instance()->registerClient(mHandler, "HistoryTestHandler");
    QTRY_VERIFY(mHandler->isRegistered());

    // register the approver
    mApprover = new Approver(this);
    History::TelepathyHelper::instance()->registerClient(mApprover, "HistoryTestApprover");
    // Tp-qt does not set registered status to approvers
    QTRY_VERIFY(QDBusConnection::sessionBus().interface()->isServiceRegistered(TELEPHONY_SERVICE_APPROVER));

    // we need to wait in order to give telepathy time to notify about the approver and handler
    QTest::qWait(3000);
}

void DaemonTest::init()
{
    mAccount = addAccount("mock", "mock", "the account");
    QVERIFY(!mAccount.isNull());
    QTRY_VERIFY(mAccount->isReady(Tp::Account::FeatureCore));
    QTRY_VERIFY(!mAccount->connection().isNull());
    QTRY_VERIFY(History::TelepathyHelper::instance()->connected());

    mMockController = new MockController("mock", this);
}

void DaemonTest::cleanup()
{
    doCleanup();
    mMockController->deleteLater();
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
    properties["Recipients"] = QStringList() << sender;

    QSignalSpy handlerSpy(mHandler, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    mMockController->placeIncomingMessage(message, properties);
    QTRY_COMPARE(threadsAddedSpy.count(), 1);
    History::Threads threads = threadsAddedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread thread = threads.first();

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
 
void DaemonTest::testMessageSentNoEventId()
{
    // Request the contact to start chatting to
    QSignalSpy spy(this, SIGNAL(contactsReceived(QList<Tp::ContactPtr>)));

    QString recipient = "11111111";

    connect(mAccount->connection()->contactManager()->contactsForIdentifiers(QStringList() << recipient),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPendingContactsFinished(Tp::PendingOperation*)));

    QTRY_COMPARE(spy.count(), 1);

    QList<Tp::ContactPtr> contacts = spy.first().first().value<QList<Tp::ContactPtr> >();
    QCOMPARE(contacts.count(), 1);
    QCOMPARE(contacts.first()->id(), recipient);

    QSignalSpy spyTextChannel(mHandler, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    Q_FOREACH(Tp::ContactPtr contact, contacts) {
        mAccount->ensureTextChat(contact, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".HistoryTestHandler");
    }
    QTRY_COMPARE(spyTextChannel.count(), 1);

    Tp::TextChannelPtr channel = spyTextChannel.first().first().value<Tp::TextChannelPtr>();
    QVERIFY(channel);

    QSignalSpy eventsAddedSpy(History::Manager::instance(), SIGNAL(eventsAdded(History::Events)));

    QString messageText = "Hello, big world!";
    Tp::Message m(Tp::ChannelTextMessageTypeNormal, messageText);
    Tp::MessagePartList parts = m.parts();
    Tp::MessagePart header = parts[0];
    Tp::MessagePart body = parts[1];
    header["no-event-id"] = QDBusVariant(true);
    Tp::MessagePartList newPart;
    newPart << header << body;
    Tp::PendingSendMessage *message = channel->send(newPart);

    QTRY_COMPARE(eventsAddedSpy.count(), 1);
    History::Events events = eventsAddedSpy.first().first().value<History::Events>();
    QCOMPARE(events.count(), 1);
    History::TextEvent event = events.first();
    QVERIFY(!event.eventId().isEmpty());

    channel->requestClose();
}

void DaemonTest::testMessageSent()
{
    // Request the contact to start chatting to
    QSignalSpy spy(this, SIGNAL(contactsReceived(QList<Tp::ContactPtr>)));

    QString recipient = "987654321";

    connect(mAccount->connection()->contactManager()->contactsForIdentifiers(QStringList() << recipient),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPendingContactsFinished(Tp::PendingOperation*)));

    QTRY_COMPARE(spy.count(), 1);

    QList<Tp::ContactPtr> contacts = spy.first().first().value<QList<Tp::ContactPtr> >();
    QCOMPARE(contacts.count(), 1);
    QCOMPARE(contacts.first()->id(), recipient);

    QSignalSpy spyTextChannel(mHandler, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    Q_FOREACH(Tp::ContactPtr contact, contacts) {
        mAccount->ensureTextChat(contact, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".HistoryTestHandler");
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
    mMockController->placeCall(properties);
    QTRY_COMPARE(newCallSpy.count(), 1);

    QSignalSpy threadsAddedSpy(History::Manager::instance(), SIGNAL(threadsAdded(History::Threads)));
    QSignalSpy threadsModifiedSpy(History::Manager::instance(), SIGNAL(threadsModified(History::Threads)));
    QSignalSpy eventsAddedSpy(History::Manager::instance(), SIGNAL(eventsAdded(History::Events)));

    // now hangup the call and check that the event was added to the database
    mMockController->hangupCall(callerId);

    QTRY_COMPARE(threadsAddedSpy.count(), 1);
    History::Threads threads = threadsAddedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread thread = threads.first();

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
    QString phoneNumber = "44444444";
    QSignalSpy spy(this, SIGNAL(contactsReceived(QList<Tp::ContactPtr>)));

    connect(mAccount->connection()->contactManager()->contactsForIdentifiers(QStringList() << phoneNumber),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPendingContactsFinished(Tp::PendingOperation*)));
    QVERIFY(spy.wait());

    QList<Tp::ContactPtr> contacts = spy.first().first().value<QList<Tp::ContactPtr> >();
    QCOMPARE(contacts.count(), 1);
    QCOMPARE(contacts.first()->id(), phoneNumber);

    QSignalSpy spyCallChannel(mHandler, SIGNAL(callChannelAvailable(Tp::CallChannelPtr)));

    Q_FOREACH(Tp::ContactPtr contact, contacts) {
        mAccount->ensureAudioCall(contact, "audio", QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".HistoryTestHandler");
    }
    QVERIFY(spyCallChannel.wait());

    Tp::CallChannelPtr channel = spyCallChannel.first().first().value<Tp::CallChannelPtr>();
    QVERIFY(channel);

    mMockController->setCallState(phoneNumber, "alerting");
    QTRY_COMPARE(channel->callState(), Tp::CallStateInitialised);

    mMockController->setCallState(phoneNumber, "active");
    QTRY_COMPARE(channel->callState(), Tp::CallStateActive);

    QSignalSpy threadsAddedSpy(History::Manager::instance(), SIGNAL(threadsAdded(History::Threads)));
    QSignalSpy threadsModifiedSpy(History::Manager::instance(), SIGNAL(threadsModified(History::Threads)));
    QSignalSpy eventsAddedSpy(History::Manager::instance(), SIGNAL(eventsAdded(History::Events)));

    mMockController->setCallState(phoneNumber, "disconnected");
    QTRY_COMPARE(channel->callState(), Tp::CallStateEnded);

    QTRY_COMPARE(threadsAddedSpy.count(), 1);
    History::Threads threads = threadsAddedSpy.first().first().value<History::Threads>();
    QCOMPARE(threads.count(), 1);
    History::Thread thread = threads.first();

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

void DaemonTest::testDeliveryReport_data()
{
    QTest::addColumn<QString>("phoneNumber");
    QTest::addColumn<QString>("deliveryStatus");
    QTest::addColumn<History::MessageStatus>("messageStatus");

    QTest::newRow("delivered status") << "11112222" << "delivered" << History::MessageStatusDelivered;
    QTest::newRow("temporarily failed") << "11113333" << "temporarily_failed" << History::MessageStatusTemporarilyFailed;
    QTest::newRow("permanently failed") << "11114444" << "permanently_failed" << History::MessageStatusPermanentlyFailed;
    QTest::newRow("accepted status") << "11115555" << "accepted" << History::MessageStatusAccepted;
    QTest::newRow("read status") << "11116666" << "read" << History::MessageStatusRead;
    QTest::newRow("deleted") << "11117777" << "deleted" << History::MessageStatusDeleted;
    QTest::newRow("unknown") << "11118888" << "unknown" << History::MessageStatusUnknown;
}

void DaemonTest::testDeliveryReport()
{
    QFETCH(QString, phoneNumber);
    QFETCH(QString, deliveryStatus);
    QFETCH(History::MessageStatus, messageStatus);

    // Request the contact to start chatting to
    QSignalSpy spy(this, SIGNAL(contactsReceived(QList<Tp::ContactPtr>)));

    connect(mAccount->connection()->contactManager()->contactsForIdentifiers(QStringList() << phoneNumber),
            SIGNAL(finished(Tp::PendingOperation*)),
            SLOT(onPendingContactsFinished(Tp::PendingOperation*)));

    QTRY_COMPARE(spy.count(), 1);

    QList<Tp::ContactPtr> contacts = spy.first().first().value<QList<Tp::ContactPtr> >();
    QCOMPARE(contacts.count(), 1);
    QCOMPARE(contacts.first()->id(), phoneNumber);

    QSignalSpy spyTextChannel(mHandler, SIGNAL(textChannelAvailable(Tp::TextChannelPtr)));

    Q_FOREACH(Tp::ContactPtr contact, contacts) {
        mAccount->ensureTextChat(contact, QDateTime::currentDateTime(), TP_QT_IFACE_CLIENT + ".HistoryTestHandler");
    }
    QTRY_COMPARE(spyTextChannel.count(), 1);

    Tp::TextChannelPtr channel = spyTextChannel.first().first().value<Tp::TextChannelPtr>();
    QVERIFY(channel);

    QSignalSpy eventsAddedSpy(History::Manager::instance(), SIGNAL(eventsAdded(History::Events)));

    QString messageText = "Hello, big world!";
    Tp::PendingSendMessage *message = channel->send(messageText);

    QTRY_COMPARE(eventsAddedSpy.count(), 1);
    History::Events events = eventsAddedSpy.first().first().value<History::Events>();
    QCOMPARE(events.count(), 1);
    History::TextEvent event = events.first();

    // now send a delivery report for this text and make sure the event gets updated
    QSignalSpy eventsModifiedSpy(History::Manager::instance(), SIGNAL(eventsModified(History::Events)));

    mMockController->placeDeliveryReport(QStringList() << phoneNumber, event.eventId(), deliveryStatus);
    QTRY_COMPARE(eventsModifiedSpy.count(), 1);
    events = eventsModifiedSpy.first().first().value<History::Events>();
    QCOMPARE(events.count(), 1);
    event = events.first();
    QCOMPARE(event.messageStatus(), messageStatus);

    channel->requestClose();
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
