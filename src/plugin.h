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

#ifndef HISTORY_PLUGIN_H
#define HISTORY_PLUGIN_H

#include <QtPlugin>
#include "types.h"

namespace History
{

class Writer;
class Reader;

class Plugin
{
public:
    virtual ~Plugin() {}
    virtual WriterPtr writer() const = 0;
    virtual ReaderPtr reader() const = 0;
};

}

Q_DECLARE_INTERFACE(History::Plugin, "com.canonical.historyservice.Plugin")

#endif // HISTORY_PLUGIN_H
