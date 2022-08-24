import QtQuick 2.12
import QtQuick.Window 2.1
import Neobox 1.0

Window {
  id: backbody
  flags: Qt.Window | Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint
  signal grabFinished(string str)
  // opacity: 0.5

  PixmapImage {
    id: pixmapImage
    anchors.fill: parent
  }

  function setBackgroundImage(picmap) {
    pixmapImage.setImage(picmap)
  }

  Canvas {
    id: canvas
    focus: true
    Keys.enabled: true

    property int m_TextLeft: 0
    property int m_TextWidth: 0
    property int m_TextTop: 0
    property int m_TextHeight: 0

    anchors.fill: parent

    onPaint: {
      var ctx = getContext("2d")
      ctx.reset()
      ctx.strokeStyle = "red"
      ctx.rect(m_TextLeft, m_TextTop, m_TextWidth, m_TextHeight)
      ctx.stroke()
    }

    MouseArea {
      anchors.fill: parent

      onPressed: {
        if (mouse.button == Qt.LeftButton) {
          parent.m_TextLeft = mouseX
          parent.m_TextTop = mouseY
        }
      }

      onReleased: {
        if (mouse.button == Qt.LeftButton) {
          backbody.close()
          backbody.grabFinished(pixmapImage.getText(
            parent.m_TextLeft, parent.m_TextTop,
            parent.m_TextWidth, parent.m_TextHeight
          ))
        }
      }

      onPositionChanged: {
        if (mouse.buttons == Qt.LeftButton) {
          parent.m_TextWidth = mouseX - parent.m_TextLeft
          parent.m_TextHeight = mouseY - parent.m_TextTop
          parent.requestPaint()
        }
      }

    }

    Keys.onPressed: {
      switch(event.key) {
        case Qt.Key_Escape:
          event.accepted = true
          backbody.close()
          break
      }
    }
  }
}
