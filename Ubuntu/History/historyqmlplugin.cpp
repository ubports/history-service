#include "historyqmlplugin.h"
#include "historyqmlfilter.h"
#include "historyqmlintersectionfilter.h"
#include "historyqmlunionfilter.h"
#include "historythreadmodel.h"
#include "historyeventmodel.h"
#include <QQmlEngine>
#include <qqml.h>

void HistoryQmlPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    // FIXME: check what to do here
    Q_UNUSED(engine)
    Q_UNUSED(uri)
}

void HistoryQmlPlugin::registerTypes(const char *uri)
{
    // @uri History
    qmlRegisterType<HistoryEventModel>(uri, 0, 1, "HistoryEventModel");
    qmlRegisterType<HistoryThreadModel>(uri, 0, 1, "HistoryThreadModel");
    qmlRegisterType<HistoryQmlFilter>(uri, 0, 1, "HistoryFilter");
    qmlRegisterType<HistoryQmlIntersectionFilter>(uri, 0, 1, "HistoryIntersectionFilter");
    qmlRegisterType<HistoryQmlUnionFilter>(uri, 0, 1, "HistoryUnionFilter");
}
