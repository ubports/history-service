/*
 * Copyright (C) 2021 UBports Foundation
 *
 * Authors:
 *  Lionel Duboeuf <lduboeuf@ouvaton.org>
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
#include "historymanager.h"
#include "event.h"
#include "manager.h"
#include "types.h"
#include <QDebug>
#include <QJSValueList>

HistoryManager::HistoryManager(QObject *parent) :
    QObject(parent), mPendingOperation(false)
{
}

void HistoryManager::removeEvents(int eventType, const QString &maxDate, const QJSValue &callback)
{
    if (!callback.isCallable()) {
        qCritical() << "no callback found!";
        return;
    }

    QJSValue result(callback);
    if (mPendingOperation) {
        result.call(QJSValueList { 0, OperationError::OPERATION_ALREADY_PENDING });
        qWarning() << "there is a pending operation, request cancelled";
        return;
    }

    QDateTime fromDate = QDateTime::fromString(maxDate, Qt::ISODate);
    History::EventType type = (History::EventType) eventType;

    if (type == History::EventTypeNull|| !fromDate.isValid()) {
        result.call(QJSValueList { 0, OperationError::OPERATION_INVALID });
        qWarning() << "invalid type or date, request cancelled";
        return;
    }

    History::Filter queryFilter(History::FieldTimestamp, QVariant(maxDate), History::MatchLess);

    if (!queryFilter.isValid()) {
        result.call(QJSValueList { 0, OperationError::OPERATION_INVALID });
        qWarning() << "invalid filter, operation cancelled";
        return;
    }

    auto onCompleted = [this, callback](int removedCount, bool isError) {
            QJSValue result(callback);
            OperationError error = isError ? OperationError::OPERATION_FAILED : OperationError::NO_ERROR;
            result.call(QJSValueList { removedCount, error });
    };

    History::Manager::instance()->removeEvents(type, queryFilter, onCompleted);
}

int HistoryManager::getEventsCount(int eventType, const QString &maxDate)
{
    QDateTime fromDate = QDateTime::fromString(maxDate, Qt::ISODate);
    History::EventType type = (History::EventType) eventType;

    if (type == History::EventTypeNull|| !fromDate.isValid()) {
        qWarning() << "invalid type or date, request cancelled";
        return -1;
    }

    History::Filter queryFilter(History::FieldTimestamp, QVariant(maxDate), History::MatchLess);

    if (!queryFilter.isValid()) {
        qWarning() << "invalid filter, request cancelled";
        return -1;
    }

    return History::Manager::instance()->getEventsCount(type, queryFilter);
}
