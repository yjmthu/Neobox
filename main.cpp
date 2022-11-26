#include <QApplication>
#include <neoapp.h>

extern void InitApp();

int main(int argc, char* argv[]) {
  if (!glbCreateSharedMemory())
    return 0;
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  InitApp();
  a.exec();
  glbDetachSharedMemory();
  return 0;
}
