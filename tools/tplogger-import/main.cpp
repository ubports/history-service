#include <HistoryManager>
#include <HistoryThread>
#include <TextItem>
#include <VoiceItem>
#include <QCoreApplication>
#include <QDebug>
#include <TelepathyQt/Types>
#include <TelepathyLoggerQt/Init>
#include "telepathylogimporter.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    Tp::registerTypes();
    Tpl::init();
    TelepathyLogImporter importer;
    Q_UNUSED(importer)

    return app.exec();
}
