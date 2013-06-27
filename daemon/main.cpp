#include "historydaemon.h"
#include <TelepathyQt/Types>
#include <TelepathyQt/Debug>

int main(int argc, char **argv)
{
    Tp::registerTypes();
    Tp::enableWarnings(true);

    QCoreApplication app(argc, argv);

    HistoryDaemon daemon;

    return app.exec();
}
