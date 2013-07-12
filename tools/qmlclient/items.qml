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

import QtQuick 2.0
import Ubuntu.History 0.1

ListView {
    id: listView

    width: 600
    height: 800

    model: HistoryEventModel {
        type: HistoryThreadModel.EventTypeText
    }

    delegate: Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        height: 100
        border.color: "black"
        color: "lightGray"

        Column {
            anchors.fill: parent
            anchors.margins: 3
            spacing: 3

            Text {
                anchors.left: parent.left
                text: "<b>AccountId:</b> " + accountId
            }
            Text {
                anchors.left: parent.left
                text: "<b>ThreadId:</b> " + threadId
            }
            Text {
                anchors.left: parent.left
                text: "<b>Sender:</b> " + sender
            }
            Text {
                anchors.left: parent.left
                text: "<b>Message:</b> " + textMessage
            }
        }
    }
}
