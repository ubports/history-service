import QtQuick 2.0
import Ubuntu.History 0.1

ListView {
    id: listView

    width: 600
    height: 800

    model: HistoryThreadModel {
    }

    delegate: Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        border.color: "black"
        color: "lightGray"

        Row {
            anchors.fill: parent
            anchors.margins: 3
            spacing: 3

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "<b>AccountId:</b> " + accountId
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "<b>ThreadId:</b> " + threadId
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "<b>Participants:</b> " + participants
            }
        }
    }
}
