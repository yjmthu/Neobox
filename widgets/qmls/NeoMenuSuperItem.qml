import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.15

NeoMenuItem {
  haveCihld: true
  default property alias content: settings.content
  backText: "›"
  property alias exclusive: settings.exclusive
  property alias checkedChild: settings.checkedChild
  property alias curItem: settings.curItem
  property alias m_contentItem: settings.m_contentItem

  NeoMenu {
    id: settings
  }

  function show () {
    settings.show()
  }

  function hide() {
    settings.hide()
  }

  onHovereds: {
    settings.x = parent.mainWidth + x + parent.mainX
    if (settings.x + settings.width > Screen.desktopAvailableWidth)
      settings.x = parent.mainX - settings.width
    settings.y = parent.mainY + y
    if (settings.y + settings.height > Screen.desktopAvailableHeight)
      settings.y = Screen.desktopAvailableHeight - settings.height
    show()
  }

  onClicked: {
    mainwindow.hideMenu()
  }
}

