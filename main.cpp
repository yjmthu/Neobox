#include <QApplication>
#include <pluginmgr.h>
#include <QMessageBox>

// #include <winrt/Windows.Foundation.h>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  try {
    PluginMgr mgr;
    mgr.Exec();
  } catch (const std::runtime_error& error) {
    auto msg = QString::fromLocal8Bit(error.what());
    QMessageBox::critical(nullptr, "Runtime Error", msg);
#ifdef _DEBUG
  } catch (const std::logic_error& error) {
    auto msg = QString::fromLocal8Bit(error.what());
    QMessageBox::critical(nullptr, "Logic Error", msg);
#endif
  }
  return 0;
}
