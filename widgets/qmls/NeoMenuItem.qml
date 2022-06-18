import QtQuick 2.0
import QtQuick.Controls 2.12

Button {
  id: control
  leftPadding: 20
  rightPadding: 10
  topPadding: 4
  bottomPadding: 4
  anchors.topMargin: 2
  anchors.bottomMargin: 2
  anchors.rightMargin: 1
  anchors.leftMargin: 1

  background: Rectangle {
    id: rect
    radius: 4
    color: qsTr("white")
    MouseArea {
      hoverEnabled: true
      acceptedButtons: Qt.NoButton
      anchors.fill: parent
      onEntered: rect.color = "#EDEDED"
      onExited: rect.color = "transparent"
    }
  }

  contentItem: Text {
    text: control.text;
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    elide: Text.ElideRight
    color: control.down ? "blue" : "black"
    font.pointSize: 10
    font.family: "FZKai-Z03"
  }
}
