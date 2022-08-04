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
  function getText(curText, hintText, func) {
    var component = Qt.createComponent("TextDialog.qml")
    if (component.status == Component.Ready) {
      var window = component.createObject(m_contentItem, 
      {
        currentText: curText,
        hint: hintText,
        finished: func
      });
      window.visible = true
    } else if (component.status == Component.Error) {
      console.log("Error loading component:", component.errorString())
    }
  }
  NeoMenuCheckableItem {
    id: autoDownload
    text: qsTr("自动下载")

    onTriggered: {
      if (checked) {
        m_Json["auto-download"] = true
      } else {
        m_Json["auto-download"] = false
      }
      updateJson()
    }
  }
  NeoMenuItem {
    text: qsTr("图片格式")
    onTriggered: {
      getText(m_Json.imgfmt, "图片名称格式：", function(txt) {

        if (txt != m_Json.imgfmt) {
          m_Json.imgfmt = txt
          updateJson()
        }
      })
    }
  }
  NeoMenuItem {
    text: qsTr("地区选择")
    onTriggered: {
      getText(m_Json.mkt, "地区：", function(txt){
        if (txt != m_Json.mkt) {
          m_Json.mkt = txt
          updateJson()
        }
      })
    }
  }
  NeoMenuItem {
    text: qsTr("当前壁纸")
    onTriggered: {
      Qt.openUrlExternally(m_Json.images[m_Json.index].copyrightlink)
    }
  }
  NeoMenuItem {
    text: qsTr("首页测试")
    onTriggered: {
      Qt.openUrlExternally("https://www.bing.com" + m_Json.images[m_Json.index].quiz)
    }
  }

  Component.onCompleted: {
    if (m_Json["auto-download"] == undefined) {
      autoDownload.checked = false
    } else {
      autoDownload.checked = m_Json["auto-download"]
    }
  }
}
