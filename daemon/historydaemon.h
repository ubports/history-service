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

private:
    CallChannelObserver mCallObserver;
    TextChannelObserver mTextObserver;
    HistoryWriterPtr mWriter;
};

#endif
