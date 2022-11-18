#include <QApplication>
#include <QProcess>
#include <QSharedMemory>
#include <QBuffer>

#include <speedbox.h>
#include <varbox.h>
#include <appcode.hpp>

int main(int argc, char* argv[]) {

  if (!VarBox::CreateSharedMemory())
    return 0;
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  VarBox::GetInstance();
  VarBox::GetSpeedBox()->InitShow();
  a.exec();
  VarBox::DetachSharedMemory();
  return 0;
}
