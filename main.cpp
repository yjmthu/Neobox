#include <QApplication>
#include <pluginmgr.h>
#include <QMessageBox>

#include <winrt/Windows.Foundation.h>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  try {
    PluginMgr mgr;
    mgr.Exec();
  } catch (std::runtime_error err) {
    auto msg = QString::fromLocal8Bit(err.what());
    QMessageBox::critical(nullptr, "Error", msg);
  }
  return 0;
}
