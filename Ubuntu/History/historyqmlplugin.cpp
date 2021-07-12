/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of history-service.
 *
 * history-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * history-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "historyqmlplugin.h"
#include "historyqmlfilter.h"
#include "historyqmlintersectionfilter.h"
#include "historyqmlsort.h"
#include "historyqmlunionfilter.h"
#include "historythreadmodel.h"
#include "historygroupedthreadsmodel.h"
#include "historyeventmodel.h"
#include "historygroupedeventsmodel.h"
#include "historyqmltexteventattachment.h"
#include "historymanager.h"
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
    qmlRegisterType<HistoryGroupedEventsModel>(uri, 0, 1, "HistoryGroupedEventsModel");
    qmlRegisterType<HistoryThreadModel>(uri, 0, 1, "HistoryThreadModel");
    qmlRegisterType<HistoryGroupedThreadsModel>(uri, 0, 1, "HistoryGroupedThreadsModel");
    qmlRegisterType<HistoryQmlFilter>(uri, 0, 1, "HistoryFilter");
    qmlRegisterType<HistoryQmlIntersectionFilter>(uri, 0, 1, "HistoryIntersectionFilter");
    qmlRegisterType<HistoryQmlSort>(uri, 0, 1, "HistorySort");
    qmlRegisterType<HistoryQmlUnionFilter>(uri, 0, 1, "HistoryUnionFilter");
    qmlRegisterType<HistoryManager>(uri, 0, 1, "HistoryManager");
    qmlRegisterUncreatableType<HistoryQmlTextEventAttachment>(uri, 0, 1, "HistoryTextEventAttachment", "");
    qmlRegisterUncreatableType<QAbstractItemModel>(uri, 0, 1, "QAbstractItemModel", "");
}
