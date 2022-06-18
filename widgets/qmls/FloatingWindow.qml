import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.15
import Qt.labs.settings 1.0
import Qt.labs.platform 1.1

import Neobox 1.0

Window {
  id: mainwindow
  visible: true
  color: qsTr("transparent")
  width: 100
  height: 42
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool

  onActiveChanged: {
    if (!active) {
      mainmenu.visible = false;
    }
  }

  onWidthChanged: rect.width = width
  onHeightChanged: rect.height = height

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
        text: qsTr("Hide")
      }
    }
  }

  Rectangle {
    id: rect
    x: 0
    y: 0
    width: parent.width
    height: parent.height
    objectName: qsTr("rect")
    property int textindex: 0
    color: qsTr("#9655007f")
    radius: 3

    Text {
      id: memUseage
      objectName: qsTr("MemUseage")
      x: 6
      y: 6
      color: qsTr("#ffff00")
      font.family: qsTr("Carattere")
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
          mainmenu.visible = true
        } else if (mouse.button === Qt.MidButton) {
          Qt.exit(1073)
        } else if (mouse.button === Qt.LeftButton) {
          mainmenu.visible = false
        }
      }
      property point clickPos: "0,0"
      onPressed: {
        clickPos = Qt.point(mouseX, mouseY)
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
          animation.duration = width
          animation_w.duration = width
          animation.to = delta - width
          animation_w.to = 0
        } else if (mainwindow.x + mainwindow.width >= Screen.desktopAvailableWidth) {
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
        } else if (mainwindow.y + mainwindow.height >= Screen.height) {
          animation.property = qsTr("y")
          animation_w.property = qsTr("y")
          animation.duration = height
          animation_w.duration = height
          animation.to = height - delta
          animation_w.to = Screen.height - height
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
      // onFinished: {
      //     console.log("pos is (", mainwindow.x, ", ", mainwindow.y, ")")
      // }
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
      fileName: qsTr("UiSetting.ini")
      category: qsTr("Neobox")

      property alias x: rect.x
      property alias y: rect.y
      property alias color: rect.color
      property alias radius: rect.radius
    }

  }

  
  Settings {
    id: settings
    objectName: "settings"
    fileName: qsTr("UiSetting.ini")

    property bool blur: true
    property alias x: mainwindow.x
    property alias y: mainwindow.y
    property alias width: mainwindow.width
    property alias height: mainwindow.height
    property alias textindex: rect.textindex

    onBlurChanged: {
      speedBox.setRoundRect(rect.x, rect.y, rect.width, rect.height, rect.radius, blur)
    }
  }

}
