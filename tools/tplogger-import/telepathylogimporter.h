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

#ifndef TELEPATHYLOGIMPORTER_H
#define TELEPATHYLOGIMPORTER_H

#include <QObject>
#include <TelepathyLoggerQt/Types>
#include <TelepathyLoggerQt/CallEvent>
#include <TelepathyLoggerQt/TextEvent>
#include <Types>

class TelepathyLogImporter : public QObject
{
    Q_OBJECT
public:
    explicit TelepathyLogImporter(QObject *parent = 0);

public Q_SLOTS:
    void onCallEventLoaded(const Tpl::CallEventPtr &event);
    void onMessageEventLoaded(const Tpl::TextEventPtr &event);
    void onFinished();
private:
    History::WriterPtr mWriter;
    int mTextEvents;
    int mVoiceEvents;
};

#endif // TELEPATHYLOGIMPORTER_H
