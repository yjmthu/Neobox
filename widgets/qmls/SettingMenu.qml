import QtQuick 2.0


NeoMenuSuperItem {
  text: qsTr("基本设置")

  NeoMenuCheckableItem {
    text: qsTr("开机自启")
    checked: wallpaperMenu.speedMenu.appAutoStart
    onTriggered: {
      wallpaperMenu.speedMenu.appAutoStart = checked
    }
  }

  NeoMenuItem {
    text: qsTr("重启软件")
    onTriggered: {
      Qt.exit(1073)
    }
  }

  NeoMenuSuperItem {
    text: qsTr("显示模式")
    exclusive: true

    NeoMenuCheckableItem {
      text: qsTr("仅有托盘")
    }
    NeoMenuCheckableItem {
      text: qsTr("仅有窗口")
    }
    NeoMenuCheckableItem {
      text: qsTr("两者都有")
    }
    Component.onCompleted: {
      checkedChild = 2
    }
  }

}
