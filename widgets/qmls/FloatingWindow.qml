import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.15
import Qt.labs.settings 1.0

import MySpeedBox 1.0

Rectangle {
    x: 0
    y: 0
    id: rect
    objectName: qsTr("rect")
    width: 100
    height: 42
    property int textindex: 0
    color: qsTr("#9655007f")
    radius: 3

    Text {
        id: memUseage
        objectName: qsTr("MemUseage")
        x: 8
        y: 6
        color: qsTr("#ffff00")
        font.family: qsTr("Carattere Regular")
        font.pointSize: 20
        text: qsTr("0")

        Settings {
            fileName: qsTr("UiSetting.ini")
            category: qsTr("MemUseage")

            property alias x: memUseage.x
            property alias y: memUseage.y
            property alias color: memUseage.color
            property alias font: memUseage.font
        }
    }

    Text {
        id: netUpSpeed
        objectName: qsTr("NetUpSpeed")
        x: 34
        y: 4
        color: qsTr("#FAAA23")
        font.family: qsTr("Nickainley Normal")
        font.bold: true
        font.pointSize: 10
        text: qsTr("\u2191 0.0 B")

        Settings {
            fileName: qsTr("UiSetting.ini")
            category: qsTr("NetUpSpeed")

            property alias x: netUpSpeed.x
            property alias y: netUpSpeed.y
            property alias color: netUpSpeed.color
            property alias font: netUpSpeed.font
        }
    }

    Text {
        id: netDownSpeed
        objectName: qsTr("NetDownSpeed")
        x: 34
        y: 24
        color: qsTr("#8CF01E")
        font.family: qsTr("Nickainley Normal")
        font.bold: true
        font.pointSize: 10
        text: qsTr("\u2193 0.0 B")

        Settings {
            fileName: qsTr("UiSetting.ini")
            category: qsTr("NetDownSpeed")

            property alias x: netDownSpeed.x
            property alias y: netDownSpeed.y
            property alias color: netDownSpeed.color
            property alias font: netDownSpeed.font
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.PointingHandCursor
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MidButton
        onClicked: {
            if (mouse.button == Qt.RightButton) {
                speedBox.showMenu(mainwindow.x + mouse.x,
                                  mainwindow.y + mouse.y)
            } else if (mouse.button === Qt.MidButton) {
                Qt.quit()
            }
        }
        property point clickPos: "0,0"
        onPressed: {
            clickPos = Qt.point(mouse.x, mouse.y)
        }
        onReleased: {
            settings.mainwindow_x = mainwindow.x
            settings.mainwindow_y = mainwindow.y
        }

        onEntered: {
            if (rect.x != 0) {
                animation.property = qsTr("x")
                animation.to = 0
                animation.duration = rect.width
            } else if (rect.y != 0) {
                animation.property = qsTr("y")
                animation.to = 0
                animation.duration = rect.height
            } else {
                return
            }
            animation.running = true
        }

        onExited: {
            var delta = 1
            if (mainwindow.x <= 0) {
                animation.property = qsTr("x")
                animation_w.property = qsTr("x")
                animation.duration = rect.width
                animation_w.duration = rect.width
                animation.to = delta - rect.width
                animation_w.to = 0
            } else if (mainwindow.x + rect.width >= Screen.desktopAvailableWidth) {
                animation.property = qsTr("x")
                animation_w.property = qsTr("x")
                animation.duration = rect.width
                animation_w.duration = rect.width
                animation.to = rect.width - delta
                animation_w.to = Screen.desktopAvailableWidth - rect.width
            } else if (mainwindow.y <= 0) {
                animation.property = qsTr("y")
                animation_w.property = qsTr("y")
                animation.duration = rect.height
                animation_w.duration = rect.height
                animation.to = delta - rect.height
                animation_w.to = 0
            } else if (mainwindow.y + rect.height >= Screen.desktopAvailableHeight) {
                animation.property = qsTr("y")
                animation_w.property = qsTr("y")
                animation.duration = rect.height
                animation_w.duration = rect.height
                animation.to = rect.height - delta
                animation_w.to = Screen.desktopAvailableHeight - rect.height
            } else {
                return
            }
            animation.running = true
            animation_w.running = true
        }

        onPositionChanged: {
            if (mouse.buttons == Qt.LeftButton) {
                mainwindow.setX(mainwindow.x + mouse.x - clickPos.x)
                mainwindow.setY(mainwindow.y + mouse.y - clickPos.y)
            }
        }
    }

    PropertyAnimation {
        id: animation
        target: rect
        onFinished: {
            console.log("pos is (", mainwindow.x, ", ", mainwindow.y, ")")
        }
    }

    PropertyAnimation {
        id: animation_w
        target: mainwindow
    }

    SpeedBox {
        id: speedBox
    }

    Timer {
        repeat: true
        running: true
        onTriggered: {
            speedBox.updateInfo()
        }
    }

    Component.onCompleted: {
        mainwindow.x = settings.mainwindow_x
        mainwindow.y = settings.mainwindow_y
        mainwindow.width = rect.width
        mainwindow.height = rect.height
        refreshBlur()
    }

    function refreshBlur() {
      if (settings.blur) {
          speedBox.setRoundRect(x, y, width, height, radius, true)
      }
    }

    onXChanged: refreshBlur()
    onYChanged: refreshBlur()
    onRadiusChanged: refreshBlur()

    Settings {
        id: settings
        objectName: "settings"
        fileName: qsTr("UiSetting.ini")

        property int mainwindow_x: 100
        property int mainwindow_y: 100
        property bool blur: true
        property alias textindex: rect.textindex

        onBlurChanged: {
          console.log("changed!")
          speedBox.setRoundRect(rect.x, rect.y, rect.width, rect.height, rect.radius, blur)
        }
    }
    Settings {
        fileName: qsTr("UiSetting.ini")
        category: qsTr("Neobox")

        property alias x: rect.x
        property alias y: rect.y
        property alias width: rect.width
        property alias height: rect.height
        property alias color: rect.color
        property alias radius: rect.radius
    }
}
