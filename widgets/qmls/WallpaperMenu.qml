import QtQuick 2.0
import Qt.labs.settings 1.0
import Neobox 1.0

NeoMenuSuperItem {
  id: wallpaperMenu
  text: qsTr("壁纸设置")
  property SpeedMenu speedMenu: SpeedMenu {
    id: speedMenu
  }

  property Settings settings: Settings {
    id: wallpaperSetting
    fileName: qsTr("Neobox.ini")
    category: qsTr("Wallpaper")
    property alias wallpaperType: speedMenu.wallpaperType
    property alias wallpaperFirstChange: speedMenu.wallpaperFirstChange
    property alias wallpaperAutoChange: speedMenu.wallpaperAutoChange
    property alias wallpaperTimeInterval: speedMenu.wallpaperTimeInterval
  }

  NeoMenuSuperItem {
    text: qsTr("类型选择")
    exclusive: true
    checkedChild: wallpaperSetting.wallpaperType

    onCheckedChildChanged: {
      wallpaperSetting.wallpaperType = checkedChild
    }

    NeoMenuCheckableItem {
      text: qsTr("壁纸天堂")
    }
    NeoMenuCheckableItem {
      text: qsTr("必应壁纸")
    }
    NeoMenuCheckableItem {
      text: qsTr("直链壁纸")
    }
    NeoMenuCheckableItem {
      text: qsTr("本地壁纸")
    }
    NeoMenuCheckableItem {
      text: qsTr("脚本输出")
    }
  }
  NeoMenuCheckableItem {
    text: qsTr("自动更换")
    Component.onCompleted: {
      checked = wallpaperSetting.wallpaperAutoChange
    }
    onTriggered: {
      wallpaperSetting.wallpaperAutoChange = checked
    }
  }
  NeoMenuCheckableItem {
    text: qsTr("首次更换")
    Component.onCompleted: {
      checked = wallpaperSetting.wallpaperFirstChange
    }
    onTriggered: {
      wallpaperSetting.wallpaperFirstChange = checked
    }
  }
  NeoMenuItem {
    id: temp
    text: qsTr("时间间隔")
    onTriggered: {
      console.log("时间间隔")
      var component = Qt.createComponent("IntDialog.qml")
      if (component.status === Component.Ready) {
        let obj = component.createObject(temp, {
          "minVal": 5,
          "value": wallpaperSetting.wallpaperTimeInterval
        })
        // obj.closing.connect(
        //   function componentDestroy(object) {
        //     object.destroy()
        //   }
        // )
        obj.finished.connect(
          function (timeInterval) {
            wallpaperSetting.wallpaperTimeInterval = timeInterval
          }
        )
        obj.visible = true
      } else {
        console.log("出错")
      }
    }
  }
  NeoMenuItem {
    text: qsTr("存储位置")
  }
  NeoMenuItem {
    text: qsTr("清理垃圾")
    onTriggered: {
      wallpaperMenu.speedMenu.wallpaperClearJunk();
    }
  }

  WWallhaven {
    text: qsTr("更多设置")
  }
}
