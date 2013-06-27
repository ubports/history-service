#ifndef HISTORYDAEMON_H
#define HISTORYDAEMON_H

#include <QObject>
#include <QSharedPointer>
#include <Types>
#include "textchannelobserver.h"
#include "callchannelobserver.h"

class HistoryWriter;

class HistoryDaemon : public QObject
{
    Q_OBJECT
public:
    HistoryDaemon(QObject *parent = 0);
    ~HistoryDaemon();

private Q_SLOTS:
    void onObserverCreated();
    void onCallEnded(const Tp::CallChannelPtr &channel);
    void onMessageReceived(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message);
    void onMessageRead(const Tp::TextChannelPtr textChannel, const Tp::ReceivedMessage &message);
    void onMessageSent(const Tp::TextChannelPtr textChannel, const Tp::Message &message, const QString &messageToken);

private:
    CallChannelObserver mCallObserver;
    TextChannelObserver mTextObserver;
    HistoryWriterPtr mWriter;
};

#endif
