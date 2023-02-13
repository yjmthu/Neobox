#include <QApplication>
#include <pluginmgr.h>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  // try {
    PluginMgr mgr;
    mgr.Exec();
  // } catch (std::runtime_error err) {
  //   //
  // } catch (...) {
  //   //
  // }
  return 0;
}

