/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
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

#ifndef TELEPATHYLOGREADER_H
#define TELEPATHYLOGREADER_H

#include <QObject>
#include <TelepathyLoggerQt/PendingOperation>
#include <TelepathyLoggerQt/Types>
#include <TelepathyLoggerQt/LogManager>
#include <TelepathyLoggerQt/CallEvent>
#include <TelepathyLoggerQt/TextEvent>
#include <TelepathyQt/AccountManager>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QUrl>

class TelepathyLogReader : public QObject
{
    Q_OBJECT
public:
    static TelepathyLogReader *instance();

public Q_SLOTS:
    void fetchLog(const Tp::AccountPtr &account);

protected:
    void requestDatesForEntities(const Tp::AccountPtr &account, const Tpl::EntityPtrList &entities);
    void requestEventsForDates(const Tp::AccountPtr &account, const Tpl::EntityPtr &entity, const Tpl::QDateList &dates);

Q_SIGNALS:
    void loadedCallEvent(const Tpl::CallEventPtr &event);
    void loadedMessageEvent(const Tpl::TextEventPtr &event);
    void finished();

protected Q_SLOTS:
    void onAccountManagerReady(Tp::PendingOperation *op);
    void onPendingEntitiesFinished(Tpl::PendingOperation *op);
    void onPendingDatesFinished(Tpl::PendingOperation *op);
    void onPendingEventsFinished(Tpl::PendingOperation *op);

protected:
    Tpl::LogManagerPtr mLogManager;
    Tp::AccountManagerPtr mAccountManager;

private:
    explicit TelepathyLogReader(QObject *parent = 0);
    QList <Tpl::PendingOperation*> mOperations;

};

#endif // CALLLOGMODEL_H
