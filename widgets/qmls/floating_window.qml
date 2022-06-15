import QtQuick 2.0
import QtQuick.Controls 2.0
import MySpeedBox 1.0

Rectangle {
  width: 100;
  height: 42;
  color: qsTr("#9655007f");
  radius: 3;

  Text {
    objectName: qsTr("MemUseage");
    x: 8;
    y: 6;
    color: qsTr("#ffff00");
    font.family: qsTr("FZQiTi-S14S");
    font.pointSize: 20;
    text: qsTr("0");
  }

  Text {
    objectName: qsTr("NetUpSpeed");
    x: 34;
    y: 4;
    color: qsTr("#faaa23");
    font.family: qsTr("Nickainley Normal");
    font.bold: true;
    font.pointSize: 10;
    text: qsTr("\u2191 0.0 B");
  }

  Text {
    objectName: qsTr("NetDownSpeed");
    x: 34;
    y: 24;
    color: qsTr("#8cf01e");
    font.family: qsTr("Nickainley Normal");
    font.bold: true;
    font.pointSize: 10;
    text: qsTr("\u2193 0.0 B");
  }

  MouseArea {
     anchors.fill: parent;
     hoverEnabled: true;
     cursorShape: pressed ? Qt.ClosedHandCursor : Qt.PointingHandCursor;
     acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MidButton;
     onClicked: {
         if (mouse.button == Qt.RightButton) {
           speedBox.showMenu(mainwindow.x + mouse.x, mainwindow.y + mouse.y);
         } else if (mouse.button == Qt.MidButton) {
             Qt.quit();
         } else if (mouse.button == Qt.LeftButton) {
         }
     }
     property point clickPos: "0,0";
     onPressed: {
       clickPos = Qt.point(mouse.x, mouse.y);
     }
     onReleased: {
       speedBox.writePosition(mainwindow.x, mainwindow.y);
     }

     onPositionChanged: {
       if (mouse.buttons == Qt.LeftButton) {
         mainwindow.setX(mainwindow.x + mouse.x - clickPos.x);
         mainwindow.setY(mainwindow.y + mouse.y - clickPos.y);
       }
     }
  }

  SpeedBox {
    id: speedBox;
  }

  Timer {
      repeat: true;
      running: true;
      onTriggered: {
        speedBox.updateInfo();
      }
  }
}
