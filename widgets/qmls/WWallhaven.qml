import QtQuick 2.0

NeoMenuSuperItem {
  id: control
  Component.onCompleted: {
    var texts = [ qsTr("最热壁纸"), qsTr("风景壁纸"), qsTr("动漫壁纸") ]
    for (let i in texts) {
      var obj = m_component.createObject(m_contentItem, {"index": i, "text": texts[i], "checked": false})
      content.push(obj)
    }
  }
  property Component m_component: Component {
    NeoMenuSuperItem {
      id: ctl
      frontText: checked ? "✔" : "✗"
      NeoMenuItem {
        text: qsTr("启用此项")
        onTriggered: {
          ctl.checked = true
        }
      }
      NeoMenuItem {
        text: qsTr("参数设置")
        onTriggered: {
          // 
        }
      }
      NeoMenuItem {
        text: qsTr("设置路径")
        onTriggered: {
          //
        }
      }
      NeoMenuItem {
        text: qsTr("删除此项")
        onTriggered: {
          //
        }
      }
    }
  }
}
