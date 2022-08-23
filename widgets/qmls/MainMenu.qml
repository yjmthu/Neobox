import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.15
import Qt.labs.settings 1.0
import Qt.labs.platform 1.0
import Neobox 1.0

NeoMenu {
  id: mainmenu
  property SpeedMenu speedMenu: SpeedMenu {
    id: speedMenu
    property Settings settings: Settings {
      fileName: qsTr("Neobox.ini")
      category: qsTr("Wallpaper")
      property alias wallpaperType: speedMenu.wallpaperType
      property alias wallpaperFirstChange: speedMenu.wallpaperFirstChange
      property alias wallpaperAutoChange: speedMenu.wallpaperAutoChange
      property alias wallpaperTimeInterval: speedMenu.wallpaperTimeInterval
    }
  }
  NeoMenuSuperItem {
    text: qsTr("软件设置")

    SettingMenu {
      id: settingMenu
    }
    UiMenu {
      id: uiMenu
    }
    WallpaperMenu {
      id: wallpaperMenu
      text: qsTr("壁纸设置")
      speedMenu: speedMenu
    }
    NeoMenuSuperItem {
      text: qsTr("插件设置")
      NeoMenuSuperItem {
        text: qsTr("翻译设置")
        property Settings settings: Settings {
          fileName: qsTr("Neobox.ini")
          category: qsTr("Translate")
          property alias translateEnableGlobalShortcut: itemTranslateEnableGlobalShortcut.checked
          property alias translateSourceDataFolder: itemTranslateSourceDataFolder.translateSourceDataFolder
        }

        NeoMenuItem {
          id: itemTranslateSourceDataFolder
          text:qsTr("资源路径")
          property string translateSourceDataFolder: qsTr(".")

          onTriggered: {
            m_FolderDialog.currentFolder = translateSourceDataFolder
          }

          property FolderDialog m_FolderDialog: FolderDialog {
            title: qsTr("选择文件")
            options: FolderDialog.ShowDirsOnly
            onAccepted: {
              parent.translateSourceDataFolder = folder
            }
          }
        }

        NeoMenuCheckableItem {
          id: itemTranslateEnableGlobalShortcut
          checked: false
          text: qsTr("注册热键")
          property string shortcut: "Shift+A"
          onTriggered: {
            if (checked) {
              if (!speedMenu.toolOcrEnableScreenShotCut(shortcut, true))
                checked = false
            } else {
              speedMenu.toolOcrEnableScreenShotCut(shortcut, false)
            }
          }
          onShortcutChanged: {
            if (checked) wallpaperMenu.toolOcrEnableScreenShotCut(shortcut, true)
          }
          Component.onCompleted: {
            speedMenu.toolOcrEnableScreenShotCut(shortcut, true)
          }
          Component.onDestruction: {
            speedMenu.toolOcrEnableScreenShotCut(shortcut, false)
          }
        }

        NeoMenuItem {
          text: qsTr("测试功能")

          onTriggered: {
            m_ScreenFetch.setBackgroundImage(speedMenu.toolOcrGrabScreen())
            m_ScreenFetch.visibility = Window.FullScreen
          }

          property ScreenFetch m_ScreenFetch: ScreenFetch {
          }
        }
      }
    }
    NeoMenuItem {
      text: qsTr("关于软件")
    }
  }

  NeoMenuItem {
    text: qsTr("文字识别")
    onTriggered: {
      speedMenu.toolOcrGetScreenShotCut()
    }
  }

  NeoMenuSuperItem {
    text: qsTr("壁纸切换")

    NeoMenuItem {
      text: qsTr("上一张图")
      onTriggered: {
        speedMenu.wallpaperGetPrev()
      }
    }

    NeoMenuItem {
      text: qsTr("下一张图")
      onTriggered: {
        speedMenu.wallpaperGetNext()
      }
    }

    NeoMenuItem {
      text: qsTr("不看此图")
      onTriggered: {
        speedMenu.wallpaperRemoveCurrent()
      }
    }

    NeoMenuItem {
      text: qsTr("收藏图片")
    }

    NeoMenuItem {
      text: qsTr("撤销删除")
      onTriggered: {
        speedMenu.wallpaperUndoDelete();
      }
    }
  }

  NeoMenuSuperItem {
    text: qsTr("打开目录")

    NeoMenuItem {
      text: qsTr("程序目录")
      onTriggered: {
        speedMenu.appOpenAppDir()
      }
    }

    NeoMenuItem {
      text: qsTr("配置目录")
      onTriggered: {
        speedMenu.appOpenCfgDir()
      }
    }

    NeoMenuItem {
      text: qsTr("壁纸目录")
      onTriggered: {
        speedMenu.appOpenDir(speedMenu.wallpaperDir)
      }
    }
  }

  NeoMenuItem {
    text: qsTr("快速关机")
    onTriggered: {
      speedMenu.appShutdownComputer()
    }
  }

  NeoMenuItem {
    text: qsTr("快捷重启")
    onTriggered: {
      speedMenu.appRestartComputer()
    }
  }

  NeoMenuItem {
    text: qsTr("退出软件")
    onTriggered: {
      Qt.quit()
    }
  }
}

