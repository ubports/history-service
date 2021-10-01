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
#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QObject>
#include "types.h"
#include "event.h"
#include "historymodel.h"

class HistoryManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int deletedCount READ deletedCount NOTIFY deletedCountChanged)
    Q_PROPERTY(OperationError error READ error NOTIFY errorChanged)

public:

    enum OperationError {
        NO_ERROR,
        OPERATION_ALREADY_PENDING,
        OPERATION_INVALID,
        OPERATION_TIMEOUT
    };
    Q_ENUM(OperationError)

    explicit HistoryManager(QObject *parent = 0);

    int count() const;
    int deletedCount() const;
    OperationError error() const;

    Q_INVOKABLE bool removeAll(int eventType, const QString &from);

Q_SIGNALS:
    void countChanged();
    void errorChanged();
    void deletedCountChanged();
    void operationStarted();
    void operationEnded();
    void operationTimeOutReached();

protected Q_SLOTS:
    void onEventsRemoved(const History::Events &events);
    void onTimeoutReached();

private:

    void pendingOperation(bool state);
    void setCount(int count);
    void setError(OperationError error);

    History::EventViewPtr mView;
    HistoryManager::OperationError mError;
    int mCount;
    int mDeletedCount;
    bool mPendingOperation;
};

#endif // HISTORYMANAGER_H
