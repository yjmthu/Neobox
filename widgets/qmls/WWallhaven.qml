import QtQuick 2.0
import QtQuick.Controls 1.2
import Qt.labs.platform 1.0
import QtQuick.Window 2.12
import Qt.labs.qmlmodels 1.0

NeoMenuSuperItem {
  id: control
  property var m_Json: []
  property var speedMenu
  function updateJson() {
    speedMenu.wallpaperSetCurJson(JSON.stringify(m_Json))
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
          control.m_Json.WallhavenCurrent = ctl.text
          control.updateJson()
          ctl.checked = true
        }
      }
      NeoMenuItem {
        text: qsTr("参数设置")
        onTriggered: {
          m_Table.model.rows = []
          let params = control.m_Json.WallhavenApi[ctl.text]["Parameter"]
          for (let key in params) {
            m_Table.model.rows.push({
              "checked": false, 
              "checkable": true, 
              "key": key, 
              "value": params[key]
            })
          }
          m_Window.visible = true
        }
        property Window m_Window: Window {
          width: 300
          height: 300
          title: qsTr("参数设置——Neobox")
          TableView {
            id: m_Table
            anchors.fill: parent
            model: TableModel {
              TableModelColumn { display: "key" }
              TableModelColumn { display: "value" }
            }
          }
        }
      }
      NeoMenuItem {
        text: qsTr("设置路径")
        onTriggered: {
          if (ctl.checked) return
          folderDialog.currentFolder = control.m_Json.WallhavenApi[ctl.text]["Directory"]
          folderDialog.open()
        }

        property FolderDialog folderDialog: FolderDialog {
          onFolderChanged: {
            control.m_Json.WallhavenApi[ctl.text]["Directory"] = folder
            control.updateJson()
          }
        }
      }
      NeoMenuItem {
        text: qsTr("删除此项")
        onTriggered: {
          delete control.m_Json.WallhavenApi[ctl.text]
          control.updateJson()
          control.initLayout()
        }
      }
    }
  }

  function initLayout() {
    let paperData = m_Json.WallhavenApi
    let tmpIndex = 0
    m_contentItem.children = []
    for (let item in paperData) {
      var obj = m_component.createObject(m_contentItem, {"index": tmpIndex, "text": item, "checked": (m_Json.WallhavenCurrent === item)})
      content.push(obj)
      ++tmpIndex;
    }
  }

  Component.onCompleted: initLayout()
}
