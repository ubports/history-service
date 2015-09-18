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

#ifndef HISTORY_THREADVIEW_H
#define HISTORY_THREADVIEW_H

#include "types.h"
#include "filter.h"
#include "sort.h"
#include "thread.h"
#include <QObject>

namespace History
{

class ThreadViewPrivate;

class ThreadView : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ThreadView)

public:
    ThreadView(History::EventType type,
               const History::Sort &sort,
               const History::Filter &filter,
               bool grouped);
    ~ThreadView();

    Threads nextPage();
    bool isValid() const;

Q_SIGNALS:
    void threadsAdded(const History::Threads &threads);
    void threadsModified(const History::Threads &threads);
    void threadsRemoved(const History::Threads &threads);
    void invalidated();

private:
    Q_PRIVATE_SLOT(d_func(), void _d_threadsAdded(const History::Threads &threads))
    Q_PRIVATE_SLOT(d_func(), void _d_threadsModified(const History::Threads &threads))
    Q_PRIVATE_SLOT(d_func(), void _d_threadsRemoved(const History::Threads &threads))
    QScopedPointer<ThreadViewPrivate> d_ptr;

};

}

#endif // HISTORY_THREADVIEW_H
