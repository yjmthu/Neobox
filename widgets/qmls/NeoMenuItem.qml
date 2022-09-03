import QtQuick 2.0
import QtQuick.Controls 2.12

Button {
    id: control
    leftPadding: 14
    rightPadding: 14
    topPadding: 4
    bottomPadding: 4
    signal hovereds()
    signal triggered()
    property int index: -1
    property bool haveCihld: false
    property alias frontText: frontTextCtl.text
    property alias backText: backTextCtl.text

    anchors {
        topMargin: 2
        bottomMargin: 2
        rightMargin: 1
        leftMargin: 1
    }

    background: Rectangle {
        radius: 4
        color: "white"
        MouseArea {
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
            anchors.fill: parent
            onEntered: {
                parent.color = "#EDEDED"
                parent.parent.parent.changeChild(index)
                emit: control.hovereds()
            }
            onExited: {
                parent.color = "white"
            }
        }
        Text {
            id: frontTextCtl
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                topMargin: 3
                leftMargin: 2
            }
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 8
            color: control.down ? "blue" : "black"
            text: qsTr("")
        }
        Text {
            id: backTextCtl
            anchors {
                right: parent.right
                top: parent.top
                bottom: parent.bottom
                topMargin: 3
                rightMargin: 7
            }
            horizontalAlignment: Text.AlignHCenter
            font.pointSize: 8
            color: control.down ? "blue" : "black"
            text: qsTr("")
        }
    }

    contentItem: Text {
        text: control.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        color: control.down ? "blue" : "black"
        font.pointSize: 10
        // font.family: "FZKai-Z03"
        font.family: "方正楷体简体"
    }

    onClicked: {
        mainwindow.hideMenu()
        emit: triggered()
    }
}
