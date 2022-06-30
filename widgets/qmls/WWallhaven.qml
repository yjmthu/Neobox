import QtQuick 2.0

NeoMenuSuperItem {
  id: control
  property string m_JsonText
  signal updateJson()
  function initLayout() {
    let m_Json = JSON.parse(m_JsonText)
    let paperData = m_Json.WallhavenApi
    let tmpIndex = 0
    for (let item in paperData) {
      var obj = m_component.createObject(m_contentItem, {"index": tmpIndex, "text": item, "checked": (m_Json.WallhavenCurrent === item)})
      content.push(obj)
      ++tmpIndex;
    }
  }
  property Component m_component: Component {
    NeoMenuSuperItem {
      id: ctl
      frontText: checked ? "✔" : "✗"
      NeoMenuItem {
        text: qsTr("启用此项")
        onTriggered: {
          for (let i in control.content) {
            control.content[i].checked = false
          }
          let m_Json = JSON.parse(control.m_JsonText)
          m_Json.WallhavenCurrent = ctl.text
          control.m_JsonText = JSON.stringify(m_Json)
          control.updateJson()
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
