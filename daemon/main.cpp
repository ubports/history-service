#include "historydaemon.h"
#include <TelepathyQt/Types>

int main(int argc, char **argv)
{
    Tp::registerTypes();

    QCoreApplication app(argc, argv);

    HistoryDaemon daemon;

    return app.exec();
}
