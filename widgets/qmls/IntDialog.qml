import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.12

Window {
  id: dialog
  title: qsTr("请输入数字")
  property int minVal: 0
  property int maxVal: 99999
  property int value: 10
  signal finished(int val)
  Column {
    Row {
      spacing: 10
      Text {
        text: qsTr("更换壁纸的时间间隔：")
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
          Component.onCompleted: {
            text = dialog.value.toString()
          }
        }
      }
    }
    Row {
      Button {
        text: qsTr("确认")
        onClicked: {
          dialog.close()
          let reault = parseInt(textInput.text)
          if (minVal <= reault && reault <= maxVal)
            emit: finished(textInput.text)
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
