#include <QApplication>
#include <neobox/pluginmgr.h>
#include <QMessageBox>

#ifdef _WIN32
#include <windows.h>
#endif

// #include <winrt/Windows.Foundation.h>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
#ifdef _RELEASE
  try {
    auto* manager = new PluginMgr;
    manager->Exec();
  } catch (const std::runtime_error& error) {
    auto msg = QString::fromLocal8Bit(error.what());
    QMessageBox::critical(nullptr, "Runtime Error", msg);
  } catch (const std::logic_error& error) {
    auto msg = QString::fromLocal8Bit(error.what());
    QMessageBox::critical(nullptr, "Logic Error", msg);
  }
#else
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif
  auto *manager = new PluginMgr();
  manager->Exec();
#endif
  return 0;
}
