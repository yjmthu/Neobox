import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0
import Neobox 1.0

NeoMenu {
  id: mainmenu

  NeoMenuSuperItem {
    text: qsTr("软件设置")

    NeoMenuSuperItem {
      text: qsTr("基本设置")

      NeoMenuCheckableItem {
        text: qsTr("开机自启")
        checked: wallpaperMenu.speedMenu.appAutoStart
        onTriggered: {
          wallpaperMenu.speedMenu.appAutoStart = checked
        }
      }
    }
    UiMenu {
      id: uiMenu
    }
    WallpaperMenu {
      id: wallpaperMenu
    }
    NeoMenuItem {
      text: qsTr("插件设置")
    }
    NeoMenuItem {
      text: qsTr("关于软件")
    }
  }

  NeoMenuItem {
    text: qsTr("文字识别")
    onTriggered: {
      SpeedMenu.toolOcrGetScreenShotCut()
    }
  }

  NeoMenuSuperItem {
    text: qsTr("壁纸切换")

    NeoMenuItem {
      text: qsTr("上一张图")
      onTriggered: {
        wallpaperMenu.speedMenu.wallpaperGetPrev()
      }
    }

    NeoMenuItem {
      text: qsTr("下一张图")
      onTriggered: {
        wallpaperMenu.speedMenu.wallpaperGetNext()
      }
    }

    NeoMenuItem {
      text: qsTr("不看此图")
      onTriggered: {
        wallpaperMenu.speedMenu.wallpaperRemoveCurrent()
      }
    }

    NeoMenuItem {
      text: qsTr("撤销不看")
    }
  }

  NeoMenuSuperItem {
    text: qsTr("打开目录")

    NeoMenuItem {
      text: qsTr("程序目录")
      onTriggered: {
        wallpaperMenu.speedMenu.appOpenAppDir()
      }
    }

    NeoMenuItem {
      text: qsTr("配置目录")
      onTriggered: {
        wallpaperMenu.speedMenu.appOpenCfgDir()
      }
    }

    NeoMenuItem {
      text: qsTr("壁纸目录")
      onTriggered: {
        wallpaperMenu.speedMenu.appOpenDir(wallpaperMenu.speedMenu.wallpaperDir)
      }
    }
  }

  NeoMenuItem {
    text: qsTr("快速关机")
    onTriggered: {
      wallpaperMenu.speedMenu.appShutdownComputer()
    }
  }

  NeoMenuItem {
    text: qsTr("快捷重启")
    onTriggered: {
      wallpaperMenu.speedMenu.appRestartComputer()
    }
  }

  NeoMenuItem {
    text: qsTr("退出软件")
    onTriggered: {
      Qt.quit()
    }

  }
}

