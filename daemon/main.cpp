#include "historydaemon.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    HistoryDaemon daemon;

    return app.exec();
}
