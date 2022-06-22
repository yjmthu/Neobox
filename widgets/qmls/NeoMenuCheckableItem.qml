import QtQuick 2.0
import QtQuick.Controls 2.12

NeoMenuItem {
  checkable: true
  frontText: checked ? "✔" : "✗"
  onClicked: {
    mainwindow.hideMenu()
    parent.checkedChild = index
  }
}

