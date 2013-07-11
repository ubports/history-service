#ifndef HISTORY_MANAGER_P_H
#define HISTORY_MANAGER_P_H

#include <QString>
#include "types.h"

namespace History
{

class Manager;

class ManagerPrivate
{
public:
    ManagerPrivate(const QString &theBackend);
    ~ManagerPrivate();

    QString backendPlugin;
    ReaderPtr reader;
};

}

#endif // HISTORY_MANAGER_P_H
