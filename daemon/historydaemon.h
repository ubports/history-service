#ifndef HISTORYDAEMON_H
#define HISTORYDAEMON_H

#include <QObject>
#include <QSharedPointer>
#include <Types>
#include "textchannelobserver.h"

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
    TextChannelObserver mTextObserver;
    HistoryWriterPtr mWriter;
};

#endif
