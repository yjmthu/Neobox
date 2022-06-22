import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import QtQuick.Window 2.12
import QtGraphicalEffects 1.15

Window {
  id: mainmenu
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
  color: "transparent"
  width: 90
  height: content.length * 30
  property bool exclusive: false
  default property alias content: layout.children
  property alias curItem: layout.curItem
  property alias checkedChild: layout.checkedChild

  function show() {
    visible = true
  }

  function hide() {
    // 超级大坑，得先隐藏子窗口，再隐藏自己；显示的时候，先显示自己，再显示子窗口
    layout.changeChild(0)
    visible = false
  }

  Control {
    id: control
    anchors.fill: parent
    background: Rectangle {
      id: rectAngle
      anchors {
        fill: parent
        leftMargin: 3
        rightMargin: 3
        topMargin: 3
        bottomMargin: 3
      }
      radius: 5
      color: "white"
    }

    contentItem: ColumnLayout {
      id: layout
      property int curItem: -1
      property int lastCheckedChild: -1
      property int checkedChild: -1
      property alias mainX: mainmenu.x
      property alias mainY: mainmenu.y
      property alias mainWidth: mainmenu.width
      property alias mainHeight: mainmenu.height
      property alias exclusive: mainmenu.exclusive
      anchors.leftMargin: 5
      anchors.rightMargin: 5
      spacing: 3
      function changeChild (index) {
        if (0 <= curItem && curItem < children.length) {
          let item = children[curItem]
          if (item.haveCihld)
            item.hide()
        }
        curItem = index
      }
      onCheckedChildChanged: function (index) {
        if (!exclusive) return
        if (0 <= lastCheckedChild &&
            lastCheckedChild < children.length &&
            children[lastCheckedChild].checked)
        {
          children[lastCheckedChild].checked = false
        }
        lastCheckedChild = checkedChild
        if (!children[checkedChild].checked) {
          children[checkedChild].checked = true
        }
      }

      Component.onCompleted: {
        for (let i=0; i<children.length; ++i) {
          children[i].index = i;
        }
      }
    }

  }

  DropShadow {
    anchors.fill: parent
    horizontalOffset: 3
    verticalOffset: 3
    radius: 9.0
    samples: 16
    color: "#90000000"
    source: control
  }

}
