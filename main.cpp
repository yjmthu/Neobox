#include <QApplication>
#include <pluginmgr.h>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  try {
    PluginMgr mgr;
    mgr.Exec();
  } catch (std::runtime_error err) {
    //
  }
  return 0;
}
