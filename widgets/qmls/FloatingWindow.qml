import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
// import QtGraphicalEffects 1.5
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1

import Neobox 1.0

Window {
    id: mainwindow
    visible: true
    color: "transparent"
    width: 100
    height: 42
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

    property int checkedChild: 0

    function hideMenu() {
        mainmenu.hide()
    }

    function getTextCtl(index) {
        if (index === 0)
            return memUseage
        if (index === 1)
            return netUpSpeed
        if (index === 2)
            return netDownSpeed
        return null
    }

    onActiveChanged: {
        if (!active)
            hideMenu()
    }

    onActiveFocusItemChanged: {
        if (!activeFocusItem)
            hideMenu()
    }

    onWidthChanged: mainrect.width = width
    onHeightChanged: mainrect.height = height

    MainMenu {
        id: mainmenu
    }

    SystemTray {
        id: maintray
        menu: Menu {
            visible: false
            MenuItem {
                text: qsTr("Quit")
                onTriggered: Qt.quit()
            }
            MenuItem {
                text: mainwindow.visible ? qsTr("Hide") : qsTr("Show")
                onTriggered: mainwindow.visible = !mainwindow.visible
            }
        }
    }

    Rectangle {
        id: mainrect
        x: 0
        y: 0
        focus: true
        Keys.enabled: true
        width: mainwindow.width
        height: mainwindow.height
        objectName: qsTr("rect")
        property int textindex: 0
        color: "#33333377"
        radius: 3

        Text {
            id: memUseage
            x: 6
            y: 2
            color: mGradient()
            font.family: qsTr("Carattere")
            font.pointSize: 20
            text: speedBox.memUseage.toString()

            function mGradient() {
                var tmp = speedBox.memUseage
                if (tmp < 15) {
                    return "#0077FF"
                } else if (tmp < 30) {
                    return "#00FFFF"
                } else if (tmp < 50) {
                    return "#FFFF00"
                } else if (tmp < 70) {
                    return "#FF0000"
                } else if (tmp < 90) {
                    return "#FF0033"
                } else {
                    return "#FF00FF"
                }
            }

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
            x: 34
            y: 4
            color: "#FAAA23"
            font.family: qsTr("Nickainley Normal")
            font.bold: true
            font.pointSize: 10
            text: formatSpeed()

            function formatSpeed() {
                var value = speedBox.netUpSpeed
                var units = ['B', 'K', 'M', 'G', 'T', 'P', 'N']
                let unit = 0
                while (value >= 1024) {
                    value /= 1024
                    ++unit
                }
                return "↑ " + value.toFixed(1) + " " + units[unit]
            }

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
            x: 34
            y: 24
            color: "#8CF01E"
            font.family: qsTr("Nickainley Normal")
            font.bold: true
            font.pointSize: 10
            text: formatSpeed()

            function formatSpeed() {
                var value = speedBox.netDownSpeed
                var units = ['B', 'K', 'M', 'G', 'T', 'P', 'N']
                let unit = 0
                while (value >= 1024) {
                    value /= 1024
                    ++unit
                }
                return "↓ " + value.toFixed(1) + " " + units[unit]
            }

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
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.PointingHandCursor
            acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MidButton

            onClicked: {
                if (mouse.button == Qt.RightButton) {
                    mainmenu.x = mainwindow.x + mouseX
                    mainmenu.y = mainwindow.y + mouseY
                    if (mainmenu.y + mainmenu.height > Screen.desktopAvailableHeight)
                        mainmenu.y -= mainmenu.height
                    if (mainmenu.x + mainmenu.width > Screen.desktopAvailableWidth)
                        mainmenu.x = Screen.desktopAvailableWidth - mainmenu.width
                    mainmenu.show()
                } else if (mouse.button === Qt.MidButton) {
                    Qt.exit(1073)
                } else if (mouse.button === Qt.LeftButton) {
                    mainmenu.hide()
                }
            }
            property point clickPos: Qt.point(0, 0)
            onPressed: {
                clickPos = Qt.point(mouseX, mouseY)
            }

            onEntered: if (Qt.platform.os === "windows") {
                           if (mainwindow.x <= 0) {
                               animation_w.property = qsTr("x")
                               animation_w.to = 0
                               animation_w.duration = parent.width
                           } else if (mainwindow.x + width >= Screen.desktopAvailableWidth) {
                               animation_w.property = qsTr("x")
                               animation_w.to = Screen.desktopAvailableWidth - width
                               animation_w.duration = parent.width
                           } else if (mainwindow.y <= 0) {
                               animation_w.property = qsTr("y")
                               animation_w.to = 0
                               animation_w.duration = parent.height
                           } else if (mainwindow.y + height >= Screen.desktopAvailableHeight) {
                               animation_w.property = qsTr("y")
                               animation_w.to = Screen.desktopAvailableHeight - height
                               animation_w.duration = parent.height
                           } else {
                               return
                           }
                           animation_w.running = true
                       } else {
                           if (parent.x != 0) {
                               animation.property = qsTr("x")
                               animation.to = 0
                               animation.duration = parent.width
                           } else if (parent.y != 0) {
                               animation.property = qsTr("y")
                               animation.to = 0
                               animation.duration = parent.height
                           } else {
                               return
                           }
                           animation.running = true
                       }

            onExited: if (Qt.platform.os === "windows") {
                          var delta = 3
                          if (mainwindow.x <= 0) {
                              animation_w.property = qsTr("x")
                              animation_w.duration = width
                              animation_w.to = delta - width
                          } else if (mainwindow.x + mainwindow.width
                                     >= Screen.desktopAvailableWidth) {
                              animation_w.property = qsTr("x")
                              animation_w.duration = width
                              animation_w.to = Screen.desktopAvailableWidth - delta
                          } else if (mainwindow.y <= 0) {
                              animation_w.property = qsTr("y")
                              animation_w.duration = height
                              animation_w.to = delta - height
                          } else if (mainwindow.y + mainwindow.height
                                     >= Screen.desktopAvailableHeight) {
                              animation_w.property = qsTr("y")
                              animation_w.duration = height
                              animation_w.to = Screen.desktopAvailableHeight - delta
                          } else {
                              return
                          }
                          animation_w.running = true
                      } else {
                          let delta = 3
                          if (mainwindow.x <= 0) {
                              animation.property = qsTr("x")
                              animation_w.property = qsTr("x")
                              animation.duration = width
                              animation_w.duration = width
                              animation.to = delta - width
                              animation_w.to = 0
                          } else if (mainwindow.x + mainwindow.width
                                     >= Screen.desktopAvailableWidth) {
                              animation.property = qsTr("x")
                              animation_w.property = qsTr("x")
                              animation.duration = width
                              animation_w.duration = width
                              animation.to = width - delta
                              animation_w.to = Screen.desktopAvailableWidth - width
                          } else if (mainwindow.y <= 0) {
                              animation.property = qsTr("y")
                              animation_w.property = qsTr("y")
                              animation.duration = height
                              animation_w.duration = height
                              animation.to = delta - height
                              animation_w.to = 0
                          } else if (mainwindow.y + mainwindow.height
                                     >= Screen.desktopAvailableHeight) {
                              animation.property = qsTr("y")
                              animation_w.property = qsTr("y")
                              animation.duration = height
                              animation_w.duration = height
                              animation.to = height - delta
                              animation_w.to = Screen.desktopAvailableHeight - height
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

            DropArea {
                anchors.fill: parent
                onDropped: {
                    if (drop.hasUrls) {
                        mainmenu.speedMenu.wallpaperSetDrop(JSON.stringify(
                                                                drop.urls))
                    }
                }
            }
        }

        property bool shiftDown: false

        Keys.onPressed: {
            if (mainwindow.checkedChild < 0 || mainwindow.checkedChild > 3)
                return
            if (mainwindow.checkedChild == 3) {
                switch (event.key) {
                case Qt.Key_Up:
                    if (shiftDown && mainwindow.height > 1)
                        --mainwindow.height
                    event.accepted = true
                    break
                case Qt.Key_Down:
                    if (shiftDown)
                        ++mainwindow.height
                    event.accepted = true
                    break
                case Qt.Key_Left:
                    if (shiftDown && mainwindow.width > 1)
                        --mainwindow.width
                    event.accepted = true
                    break
                case Qt.Key_Right:
                    if (shiftDown)
                        ++mainwindow.width
                    event.accepted = true
                    break
                case Qt.Key_Shift:
                    shiftDown = true
                    event.accepted = true
                    break
                }
            } else {
                let obj = mainwindow.checkedChild
                    == 0 ? memUseage : (mainwindow.checkedChild == 1 ? netUpSpeed : netDownSpeed)
                switch (event.key) {
                case Qt.Key_Up:
                    if (shiftDown)
                        --obj.y
                    event.accepted = true
                    break
                case Qt.Key_Down:
                    if (shiftDown)
                        ++obj.y
                    event.accepted = true
                    break
                case Qt.Key_Left:
                    if (shiftDown)
                        --obj.x
                    event.accepted = true
                    break
                case Qt.Key_Right:
                    if (shiftDown)
                        ++obj.x
                    event.accepted = true
                    break
                case Qt.Key_Shift:
                    shiftDown = true
                    event.accepted = true
                    break
                }
            }
        }

        Keys.onReleased: {
            switch (event.key) {
            case Qt.Key_Shift:
                shiftDown = true
                event.accepted = true
                break
            }
        }

        PropertyAnimation {
            id: animation
            target: mainrect
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

        onXChanged: rectSettings.refreshBlur()
        onYChanged: rectSettings.refreshBlur()
        onRadiusChanged: rectSettings.refreshBlur()

        property Settings rectSettings: Settings {
            fileName: qsTr("UiSetting.ini")
            category: qsTr("Neobox")

            property alias x: mainrect.x
            property alias y: mainrect.y
            property alias radius: mainrect.radius
            property alias color: mainrect.color

            function refreshBlur() {
                if (settings.blur) {
                    speedBox.setRoundRect(x, y, width, height, radius, true)
                } else {
                    speedBox.setRoundRect(0, 0, 0, 0, 0, false)
                }
            }
            Component.onCompleted: {
                refreshBlur()
            }
        }
    }

    property Settings settings: Settings {
        objectName: "settings"
        fileName: qsTr("UiSetting.ini")

        property bool blur: true
        property int flag: 0
        property alias x: mainwindow.x
        property alias y: mainwindow.y
        property alias width: mainwindow.width
        property alias height: mainwindow.height
        property alias textindex: mainrect.textindex

        onBlurChanged: {
            mainrect.rectSettings.refreshBlur()
        }
    }

    property alias mainrect: mainrect
}
