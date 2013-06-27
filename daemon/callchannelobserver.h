#ifndef CALLCHANNELOBSERVER_H
#define CALLCHANNELOBSERVER_H

#include <QObject>
#include <TelepathyQt/CallChannel>

class CallChannelObserver : public QObject
{
    Q_OBJECT
public:
    explicit CallChannelObserver(QObject *parent = 0);
    
public Q_SLOTS:
    void onCallChannelAvailable(Tp::CallChannelPtr callChannel);

Q_SIGNALS:
    void callEnded(Tp::CallChannelPtr callChannel);

protected Q_SLOTS:
    void onCallStateChanged(Tp::CallState state);

private:
    QList<Tp::CallChannelPtr> mChannels;
};

#endif // CALLCHANNELOBSERVER_H
