#include <QApplication>
#include <QProcess>
#include <QSharedMemory>
#include <QBuffer>

#include <speedbox.h>
#include <varbox.h>
#include <appcode.hpp>

static bool CreateSharedMemory() {
  static QSharedMemory qSharedMemory;
  VarBox::m_SharedMemory = &qSharedMemory;
  qSharedMemory.setKey(QStringLiteral("__Neobox__"));
  if(qSharedMemory.attach()) {
    /*
    * 0: already have one instance;
    * 1: app was restarted;
    * 2: app should go to left top.
    */
    if (VarBox::ReadSharedFlag() == 0) {
      VarBox::WriteSharedFlag(2);
      return false;
    }
  } else if (!qSharedMemory.create(sizeof(int))) {
    return false;
  }
  VarBox::WriteSharedFlag(0);
  return true;
}

int main(int argc, char* argv[]) {

  if (!CreateSharedMemory())
    return 0;
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  VarBox::GetInstance();
  VarBox::GetSpeedBox()->Show();
  a.exec();
  VarBox::m_SharedMemory->detach();
  return 0;
}
