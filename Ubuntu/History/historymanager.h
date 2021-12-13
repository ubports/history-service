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
#include <QJSValue>
#include "historymodel.h"

class HistoryManager : public QObject
{
    Q_OBJECT

public:

    enum OperationError {
        NO_ERROR,
        OPERATION_ALREADY_PENDING,
        OPERATION_INVALID,
        OPERATION_FAILED,
        OPERATION_TIMEOUT
    };
    Q_ENUM(OperationError)

    explicit HistoryManager(QObject *parent = 0);

    /*!
     * \brief removeEvents remove all events given eventType and created before maxDate,
     * \param eventType event type according to History.EventType
     * \param maxDate QString date in ISO format
     * \param callback expect a javascript function(deletedEventsCount, error)
     * deletedEventsCount number of deleted events
     * error  HistoryManager::OperationError enum type,
     */
    Q_INVOKABLE void removeEvents(int eventType, const QString &maxDate, const QJSValue &callback);

    /*!
     * \brief getEventsCount return the number of events given eventType and created before maxDate
     * \param eventType event type according to History.EventType
     * \param maxDate QString date in ISO format
     * \return number of events count
     */
    Q_INVOKABLE int getEventsCount(int eventType, const QString &maxDate);


private:
    bool mPendingOperation;
};

#endif // HISTORYMANAGER_H
