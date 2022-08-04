import QtQuick 2.14
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.1
import Qt.labs.platform 1.0
import QtQuick.Window 2.12
import Qt.labs.qmlmodels 1.0

NeoMenuSuperItem {
  id: control
  readonly property int m_TypeSize: 1
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
          for (let i=0; i<control.content.length-m_TypeSize; ++i) {
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
          let params = control.m_Json.WallhavenApi[ctl.text]["Parameter"]
          m_Table.model.clear()
          for (let key in params) {
            m_Table.model.rows.push({
              key: key,
              val: params[key]
            })
          }
          m_Window.visible = true
        }
        property Window m_Window: Window {
          id: tableWindow
          width: 300
          height: 300
          title: qsTr("参数设置—Neobox")

          ColumnLayout {
            anchors.fill: parent

            TableView {
              id: m_Table
              columnSpacing: 1
              rowSpacing: 1
              clip: true
              Layout.fillWidth: true
              Layout.fillHeight: true

              model: TableModel {
                TableModelColumn { display: "key" }
                TableModelColumn { display: "val" }

                rows: [
                  { key: "123", val: "234" }
                ]
              }

              delegate: Rectangle {
                implicitWidth: 100
                implicitHeight: 50
                border.width: 1

                Text {
                  text: display
                  anchors.centerIn: parent
                }
              }
            }

            RowLayout {
              Button {
                text: qsTr("添加")
                Layout.fillWidth: true
                Layout.fillHeight: true

                onClicked: {
                  m_Table.model.rows.push({})
                }
              }
              Button {
                text: qsTr("删除")
                Layout.fillWidth: true
                Layout.fillHeight: true

                onClicked: {
                  //
                }
              }
              Button {
                text: qsTr("取消")
                Layout.fillWidth: true
                Layout.fillHeight: true

                onClicked: {
                  tableWindow.visible = false
                }
              }
              Button {
                text: qsTr("确定")
                Layout.fillWidth: true
                Layout.fillHeight: true

                onClicked: {
                  tableWindow.visible = false
                  let n = m_Table.model.rows.length
                  let obj = {}
                  for (let i=0; i<n; ++i) {
                    let tmp = m_Table.model.get(i)
                    obj[tmp.key] = tmp.val
                  }
                  control.m_Json.WallhavenApi[ctl.text]["Parameter"] = obj
                  control.updateJson()
                }
              }
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

  property NeoMenuItem m_PicInfo: NeoMenuItem {
    text: qsTr("关于此图")
    index: control.content.length - 2
    onTriggered: {
      let imagePath = speedMenu.wallpaperGetCurWallpaper()
      let pattern = /^.*wallhaven-([0-9a-z]{6}).(png|jpg)$/i
      imagePath.match(pattern)
      let murl = "https://wallhaven.cc/w/" + RegExp.$1
      Qt.openUrlExternally(murl)
    }
  }

  property NeoMenuItem m_PicSimilar: NeoMenuItem {
    text: qsTr("相似壁纸")
    index: control.content.length - 1
    onTriggered: {
      let imagePath = speedMenu.wallpaperGetCurWallpaper()
      let pattern = /^.*wallhaven-([0-9a-z]{6}).(png|jpg)$/i
      imagePath.match(pattern)
      let murl = "https://wallhaven.cc/search?q=like%3A" + RegExp.$1
      Qt.openUrlExternally(murl)
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
    content.push(control.m_PicInfo, control.m_PicSimilar)
    control.m_PicInfo.parent = m_contentItem
    control.m_PicSimilar.parent = m_contentItem
  }

  Component.onCompleted: initLayout()
}
