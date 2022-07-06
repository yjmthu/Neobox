import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.12

Window {
  title: qsTr("请输入文字")
  id: dialog
  property alias hint: hintTextCtl.text
  property alias currentText: textInput.text
  property string text
  property var finished

  Column {
    Row {
      spacing: 10
      Text {
        id: hintTextCtl
        horizontalAlignment: Text.AlignHCenter
      }
      Rectangle {
        width: 100
        height: 24
        border.color: "grey"

        TextInput {
          id: textInput
          anchors {
            fill: parent
            margins: 2
          }
          // inputMask: "99999"
          focus: true
        }
      }
    }
    Row {
      Button {
        text: qsTr("确认")
        onClicked: {
          dialog.text = textInput.text
          dialog.close()
          finished(dialog.text)
        }
      }
      Button {
        text: qsTr("取消")
        onClicked: {
          dialog.close()
        }
      }
    }
  }
}

