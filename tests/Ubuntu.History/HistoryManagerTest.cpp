#include <QtCore/QObject>
#include <QtTest/QtTest>
#include "manager.h"
#include "historymanager.h"
#include "voiceevent.h"

class HistoryManagerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testShouldNotTriggerOperation();
    void testRemoveAll();
private:
    History::Manager *mManager;
};

void HistoryManagerTest::initTestCase()
{
    mManager = History::Manager::instance();
}

void HistoryManagerTest::testShouldNotTriggerOperation()
{
    HistoryManager historyManager;
    QSignalSpy countChanged(&historyManager, SIGNAL(countChanged()));
    // no type
    QCOMPARE(historyManager.removeAll(), false);
    historyManager.setType(HistoryModel::EventTypeVoice);

    // no filter
    QCOMPARE(historyManager.removeAll(), false);
    HistoryQmlFilter *filter = new HistoryQmlFilter(this);
    filter->setFilterProperty(History::FieldTimestamp);
    filter->setFilterValue(QDateTime::currentDateTime().toString("yyyy-MM-ddTHH:mm:ss.zzz"));
    filter->setMatchFlags(History::MatchLess);
    historyManager.setFilter(filter);

    //start remove no datas
    QCOMPARE(historyManager.removeAll(), false);
    QTRY_COMPARE(countChanged.count(), 1);
    QCOMPARE(historyManager.count(), 0);

}

void HistoryManagerTest::testRemoveAll()
{
    HistoryManager historyManager;
    QSignalSpy countChanged(&historyManager, SIGNAL(countChanged()));
    QSignalSpy operationEnded(&historyManager, SIGNAL(operationEnded()));
    QSignalSpy operationStarted(&historyManager, SIGNAL(operationStarted()));
    QSignalSpy deletedCountChanged(&historyManager, SIGNAL(deletedCountChanged()));

    QString voiceParticipant("voiceParticipant");

    // create a temporary thread to populate the model
    History::Thread voiceThread = mManager->threadForParticipants("voiceAccountId",
                                                                  History::EventTypeVoice,
                                                                  QStringList()<< voiceParticipant,
                                                                  History::MatchCaseSensitive, true);

    History::VoiceEvent voiceEvent(voiceThread.accountId(),
                                   voiceThread.threadId(),
                                   QString("eventId1"),
                                   voiceParticipant,
                                   QDateTime::currentDateTime(),
                                   true,
                                   true);
    QVERIFY(mManager->writeEvents(History::Events() << voiceEvent));

    HistoryQmlFilter *filter = new HistoryQmlFilter(this);
    filter->setFilterProperty(History::FieldTimestamp);
    filter->setFilterValue(QDateTime::currentDateTime().addDays(1).toString("yyyy-MM-ddTHH:mm:ss.zzz"));
    filter->setMatchFlags(History::MatchLess);

    historyManager.setType(HistoryModel::EventTypeVoice);
    historyManager.setFilter(filter);

    QCOMPARE(historyManager.eventsCount(), 1);

    QVERIFY(historyManager.removeAll());
    QTRY_COMPARE(operationStarted.count(), 1);
    QTRY_COMPARE(operationEnded.count(), 1);
    QCOMPARE(historyManager.deletedCount(), 1);
}


QTEST_MAIN(HistoryManagerTest)
#include "HistoryManagerTest.moc"
