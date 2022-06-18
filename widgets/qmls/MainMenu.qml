import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.15

import Neobox 1.0

Window {
  id: mainmenu
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
  color: "transparent"

  Control {
    id: control
    anchors.fill: parent
    padding: 5

    background: Rectangle {
      id: rect
      anchors.fill: parent
      radius: 5
      color: "white"
    }

    contentItem: ColumnLayout {
      // spacing: 5
      anchors.centerIn: parent

      NeoMenuItem {
        text: qsTr("软件设置")
      }

      NeoMenuItem {
        text: qsTr("文字识别")
        onClicked: {
          mainmenu.visible = false
          SpeedMenu.toolOcrGetScreenShotCut()
        }
      }

      NeoMenuItem {
        text: qsTr("上一张图")
        onClicked: {
          mainmenu.visible = false
          speedMenu.wallpaperGetPrev()
        }
      }

      NeoMenuItem {
        text: qsTr("下一张图")
        onClicked: {
          mainmenu.visible = false
          speedMenu.wallpaperGetNext()
        }
      }

      NeoMenuItem {
        text: qsTr("不看此图")
        onClicked: {
          mainmenu.visible = false
          speedMenu.wallpaperRemoveCurrent()
        }
      }

      NeoMenuItem {
        text: qsTr("程序目录")
        onClicked: {
          mainmenu.visible = false
          speedMenu.appOpenDir(Qt.application.arguments[0]);
        }
      }

      NeoMenuItem {
        text: qsTr("快速关机")
        onClicked: {
          mainmenu.visible = false
          speedMenu.appShutdownComputer()
        }
      }

      NeoMenuItem {
        text: qsTr("快捷重启")
        onClicked: {
          mainmenu.visible = false
          speedMenu.appRestartComputer()
        }
      }

      NeoMenuItem {
        text: qsTr("退出软件")
        onClicked: {
          mainmenu.visible = false
          Qt.quit()
        }
      }


      Component.onCompleted: {
        mainmenu.width = width + control.padding*2
        mainmenu.height = height + control.padding*2
        console.log(mainmenu.width, ", ", mainmenu.height)
      }

    }

    SpeedMenu {
      id: speedMenu
    }
  }
}
