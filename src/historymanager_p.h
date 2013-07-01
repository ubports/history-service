#ifndef HISTORYMANAGER_P_H
#define HISTORYMANAGER_P_H

#include <QString>
#include <Types>

class HistoryManager;

class HistoryManagerPrivate
{
    Q_DECLARE_PUBLIC(HistoryManager)
public:
    HistoryManagerPrivate(const QString &theBackend);
    ~HistoryManagerPrivate();

    QString backendPlugin;

    HistoryManager *q_ptr;
    HistoryReaderPtr reader;
};

#endif // HISTORYMANAGER_P_H
