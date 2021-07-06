/*
 * Copyright (C) 2021 Ubports Foundation
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


HistoryManager::HistoryManager(QObject *parent) : QObject(parent), mFilter(0), mSort(new HistoryQmlSort(this)), mType(HistoryModel::EventTypeText), mDeletedCount(0), mPendingOperation(false), mSignalsTimer(-1)
{
}

HistoryQmlFilter *HistoryManager::filter() const
{
    return mFilter;
}

void HistoryManager::setFilter(HistoryQmlFilter *value)
{
    if (mFilter) {
        mFilter->disconnect(this);
    }

    mFilter = value;

    Q_EMIT filterChanged();
}

HistoryModel::EventType HistoryManager::type() const
{
    return mType;
}

void HistoryManager::setType(HistoryModel::EventType value)
{
    mType = value;
    Q_EMIT typeChanged();
}

HistoryQmlSort *HistoryManager::sort() const
{
    return mSort;
}

int HistoryManager::count() const
{
    return mCount;
}

int HistoryManager::deletedCount() const
{
    return mDeletedCount;
}

void HistoryManager::setSort(HistoryQmlSort *value)
{
    // disconnect the previous sort
    if (mSort) {
        mSort->disconnect(this);
    }

    mSort = value;

    Q_EMIT sortChanged();
}

int HistoryManager::eventsCount()
{
    History::Filter queryFilter;

    if (!mType || !mFilter) {
        return false;
    }

    if (mFilter && mFilter->filter().isValid()) {
        queryFilter = mFilter->filter();
    }

    return History::Manager::instance()->eventsCount((History::EventType)mType, queryFilter);
}

void HistoryManager::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == mSignalsTimer) {
        pendingOperation(false);
    }
}

bool HistoryManager::removeAll()
{

    History::Filter queryFilter;
    History::Sort querySort;

    if (mPendingOperation) {
        qWarning() << "there is a pending operation, request cancelled";
        return false;
    }

    if (!mType || !mFilter || !mFilter->filter().isValid()) {
        qWarning() << "type or filter not set, operation cancelled";
        return false;
    }

    if (!mView.isNull()) {
        mView->disconnect(this);
    }

    queryFilter = mFilter->filter();

    if (mSort) {
        querySort = mSort->sort();
    }

    setCount(eventsCount());
    mDeletedCount = 0;

    bool started = false;
    if (count() > 0) {

        mView = History::Manager::instance()->queryEvents((History::EventType)mType, querySort, queryFilter);
        connect(mView.data(),
                SIGNAL(eventsRemoved(History::Events)),
                SLOT(onEventsRemoved(History::Events)));

        started = History::Manager::instance()->removeEvents((History::EventType)mType, queryFilter, querySort);

    }
    pendingOperation(started);

    return started;
}

void HistoryManager::onEventsRemoved(const History::Events &events)
{
    mDeletedCount += events.count();
    qDebug() << "mDeletedCount" << mDeletedCount;
    Q_EMIT deletedCountChanged();

    if (mDeletedCount == mCount) {
        pendingOperation(false);
    }
}

void HistoryManager::pendingOperation(bool state)
{
    mPendingOperation = state;
    if (mPendingOperation) {
        if (mSignalsTimer >= 0) {
            killTimer(mSignalsTimer);
        }

        mSignalsTimer = startTimer(180000);
        Q_EMIT operationStarted();
    } else {
        if (mSignalsTimer >= 0) {
            killTimer(mSignalsTimer);
            mSignalsTimer = -1;
        }
        Q_EMIT operationEnded();
    }
}

void HistoryManager::setCount(int count)
{
    mCount = count;
    Q_EMIT countChanged();
}
