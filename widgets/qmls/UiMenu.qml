import QtQuick 2.2
import QtQuick.Dialogs 1.3

NeoMenuSuperItem {
  text: qsTr("外观设置")
  NeoMenuSuperItem {
    text: qsTr("矩形窗口")
    NeoMenuCheckableItem {
      text: qsTr("鼠标穿透")
    }
    NeoMenuCheckableItem {
      text: qsTr("模糊效果")
      checked: mainwindow.settings.blur
      onTriggered: {
        mainwindow.settings.blur = checked
      }
    }
    NeoMenuItem {
      text: qsTr("背景颜色")
      property ColorDialog colorDialog: ColorDialog {
        title: "Please choose a color"
        showAlphaChannel: true
        onAccepted: {
          mainwindow.mainrect.rectSettings.color = color
          visible = false
        }
        onRejected: {
          visible = false
        }
      }
      onTriggered: {
        var temp = mainwindow.mainrect.rectSettings
        colorDialog.currentColor = temp.color
        colorDialog.visible = true
      }
    }

    NeoMenuItem {
      text: qsTr("界面大小")
    }
    NeoMenuSuperItem {
      text: qsTr("窗口属性")
      exclusive: true
      onCheckedChildChanged: {
        mainwindow.settings.flag = checkedChild
      }
      NeoMenuCheckableItem {
        text: qsTr("顶部")
        onCheckedChanged: {
          if (!checked) return
          mainwindow.flags = Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
        }
      }
      NeoMenuCheckableItem {
        text: qsTr("普通")
        onCheckedChanged: {
          if (!checked) return
          mainwindow.flags = Qt.FramelessWindowHint | Qt.Tool
        }
      }
      NeoMenuCheckableItem {
        text: qsTr("底部")
        onCheckedChanged: {
          if (!checked) return
          mainwindow.flags = Qt.FramelessWindowHint | Qt.WindowStaysOnBottomHint | Qt.Tool
        }
      }
      Component.onCompleted: {
        checkedChild = mainwindow.settings.flag;
      }
    }
  }
  NeoMenuSuperItem {
    text: qsTr("界面文字")
    NeoMenuSuperItem {
      text: qsTr("文字选择")
      exclusive: true
      NeoMenuCheckableItem {
        text: qsTr("内存占用")
      }
      NeoMenuCheckableItem {
        text: qsTr("上传速度")
      }
      NeoMenuCheckableItem {
        text: qsTr("下载速度")
      }
      Component.onCompleted: {
        checkedChild = 0;
      }
    }
    NeoMenuItem {
      text: qsTr("文字颜色")
    }
    NeoMenuItem {
      text: qsTr("字体大小")
    }
    NeoMenuItem {
      text: qsTr("字体颜色")
    }
  }
}

