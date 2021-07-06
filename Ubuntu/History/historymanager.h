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
#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QObject>
#include <QTimerEvent>
#include "types.h"
#include "event.h"
#include "historyqmlfilter.h"
#include "historyqmlsort.h"
#include "historymodel.h"

class HistoryManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(HistoryQmlFilter *filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(HistoryQmlSort *sort READ sort WRITE setSort NOTIFY sortChanged)
    Q_PROPERTY(HistoryModel::EventType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int deletedCount READ deletedCount NOTIFY deletedCountChanged)

public:

    explicit HistoryManager(QObject *parent = 0);

    HistoryQmlFilter *filter() const;
    void setFilter(HistoryQmlFilter *value);

    HistoryModel::EventType type() const;
    void setType(HistoryModel::EventType value);

    void setSort(HistoryQmlSort *value);
    HistoryQmlSort *sort() const;

    int count() const;
    int deletedCount() const;

    Q_INVOKABLE bool removeAll();
    Q_INVOKABLE int eventsCount();

protected:
    void timerEvent(QTimerEvent *event) override;

Q_SIGNALS:
    void filterChanged();
    void sortChanged();
    void typeChanged();
    void countChanged();
    void deletedCountChanged();
    void operationStarted();
    void operationEnded();

protected Q_SLOTS:
    void onEventsRemoved(const History::Events &events);

private:

    void pendingOperation(bool state);
    void setCount(int count);

    HistoryQmlFilter *mFilter;
    HistoryQmlSort *mSort;
    HistoryModel::EventType mType;
    History::EventViewPtr mView;
    int mCount;
    int mDeletedCount;
    bool mPendingOperation;
    int mSignalsTimer;
};

#endif // HISTORYMANAGER_H
