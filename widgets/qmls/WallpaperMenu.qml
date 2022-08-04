import QtQuick 2.0
import Qt.labs.platform 1.0
import Neobox 1.0

NeoMenuSuperItem {
  id: wallpaperMenu
  property var speedMenu
  property var moreOptions

  NeoMenuSuperItem {
    text: qsTr("类型选择")
    exclusive: true

    onCheckedChildChanged: {
      if (speedMenu.wallpaperType != checkedChild) {
        speedMenu.wallpaperType = checkedChild
        wallpaperMenu.changeWallpaperType(checkedChild)
      }
    }

    NeoMenuCheckableItem {
      text: qsTr("壁纸天堂")
      onTriggered: {
      }
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
    NeoMenuCheckableItem {
      text: qsTr("收藏夹")
    }
    Component.onCompleted: {
      checkedChild = speedMenu.wallpaperType
    }
  }
  NeoMenuCheckableItem {
    text: qsTr("自动更换")
    Component.onCompleted: {
      checked = speedMenu.wallpaperAutoChange
    }
    onTriggered: {
      speedMenu.wallpaperAutoChange = checked
    }
  }
  NeoMenuCheckableItem {
    text: qsTr("首次更换")
    Component.onCompleted: {
      checked = speedMenu.wallpaperFirstChange
    }
    onTriggered: {
      speedMenu.wallpaperFirstChange = checked
    }
  }
  NeoMenuItem {
    id: temp
    text: qsTr("时间间隔")
    onTriggered: {
      var component = Qt.createComponent("IntDialog.qml")
      if (component.status === Component.Ready) {
        let obj = component.createObject(temp, {
          minVal: 5,
          value: speedMenu.wallpaperTimeInterval
        })
        obj.finished.connect(
          function (timeInterval) {
            speedMenu.wallpaperTimeInterval = timeInterval
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
    onTriggered: {
      m_FolderDialog.currentFolder = speedMenu.wallpaperDir
      m_FolderDialog.open()
    }
    property FolderDialog m_FolderDialog: FolderDialog {
      title: qsTr("选择文件")
      options: FolderDialog.ShowDirsOnly
      onAccepted: {
        speedMenu.wallpaperDir = folder
      }
    }
  }
  NeoMenuItem {
    text: qsTr("清理垃圾")
    onTriggered: {
      wallpaperMenu.speedMenu.wallpaperClearJunk();
    }
  }
  NeoMenuItem {
    text: qsTr("更多设置")
  }

  Component.onCompleted: {
    changeWallpaperType(wallpaperMenu.speedMenu.wallpaperType)
  }
  function changeWallpaperType(typeIndex) {
    --content.length
    var component
    switch (typeIndex) {
    case 0:
      component = Qt.createComponent("WWallhaven.qml")
      break
    case 1:
      component = Qt.createComponent("WBing.qml")
      break
    case 2:
    case 3:
    case 4:
    case 5:
    default:
      return
    }
    if (component.status == Component.Ready) {
      moreOptions = component.createObject(m_contentItem, 
      {
        text: qsTr("更多设置"),
        index: m_contentItem.children.length,
        m_Json: JSON.parse(wallpaperMenu.speedMenu.wallpaperGetCurJson()),
        speedMenu: wallpaperMenu.speedMenu
      });
      content.push(moreOptions)
    } else if (component.status == Component.Error) {
      console.log("Error loading component:", component.errorString())
    }
  }
}
