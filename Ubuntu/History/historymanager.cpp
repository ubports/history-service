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
#include "manager.h"
#include "eventview.h"
#include <QDebug>
#include <QTimer>


HistoryManager::HistoryManager(QObject *parent) : QObject(parent), mError(OperationError::NO_ERROR), mDeletedCount(0), mPendingOperation(false)
{
}

int HistoryManager::count() const
{
    return mCount;
}

int HistoryManager::deletedCount() const
{
    return mDeletedCount;
}

HistoryManager::OperationError HistoryManager::error() const
{
    return mError;
}

bool HistoryManager::removeAll(int eventType, const QString &from)
{

    if (mPendingOperation) {
        setError(OperationError::OPERATION_ALREADY_PENDING);
        qWarning() << "there is a pending operation, request cancelled";
        return false;
    }

    QDateTime fromDate = QDateTime::fromString(from, Qt::ISODate);
    History::EventType type = (History::EventType) eventType;

    if (type == History::EventTypeNull|| !fromDate.isValid()) {
        setError(OperationError::OPERATION_INVALID);
        qWarning() << "invalid type or date, request cancelled";
        return false;
    }

    History::Filter queryFilter(History::FieldTimestamp, QVariant(from), History::MatchLess);
    History::Sort querySort(History::FieldTimestamp, Qt::DescendingOrder);

    if (!queryFilter.isValid()) {
        setError(OperationError::OPERATION_INVALID);
        qWarning() << "filter invalid, operation cancelled";
        return false;
    }

    if (!mView.isNull()) {
        mView->disconnect(this);
    }

    setCount(History::Manager::instance()->eventsCount(type, queryFilter));
    mDeletedCount = 0;
    setError(OperationError::NO_ERROR);

    bool started = false;
    if (count() > 0) {

        mView = History::Manager::instance()->queryEvents(type, querySort, queryFilter);
        connect(mView.data(),
                SIGNAL(eventsRemoved(History::Events)),
                SLOT(onEventsRemoved(History::Events)));

        History::Manager::instance()->removeEvents(type, queryFilter, querySort);
        started = true;

    }
    pendingOperation(started);

    return started;
}

void HistoryManager::onEventsRemoved(const History::Events &events)
{
    mDeletedCount += events.count();
    qDebug() << "onEventsRemoved: removed " << mDeletedCount << " events";
    Q_EMIT deletedCountChanged();

    if (mDeletedCount == mCount) {
        pendingOperation(false);
    }
}

void HistoryManager::onTimeoutReached()
{
    if (mPendingOperation) {
        pendingOperation(false);
        setError(OperationError::OPERATION_TIMEOUT);
        Q_EMIT operationTimeOutReached();
    }
}

void HistoryManager::pendingOperation(bool state)
{
    mPendingOperation = state;
    if (mPendingOperation) {
        QTimer::singleShot(180000, this, &HistoryManager::onTimeoutReached);
        Q_EMIT operationStarted();
    } else {
        setError(OperationError::NO_ERROR);
        Q_EMIT operationEnded();
    }
}

void HistoryManager::setCount(int count)
{
    mCount = count;
    Q_EMIT countChanged();
}

void HistoryManager::setError(OperationError error)
{
    mError = error;
    Q_EMIT errorChanged();
}
