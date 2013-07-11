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
