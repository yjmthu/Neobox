#include <QApplication>
#include <QProcess>
#include <QSharedMemory>

#include <speedbox.h>
#include <varbox.h>
#include <appcode.hpp>

int main(int argc, char* argv[]) {
  QSharedMemory qSharedMemory;
  qSharedMemory.setKey(QStringLiteral("__Neobox__"));
  if(qSharedMemory.attach() || !qSharedMemory.create(1))
    return 0;
  VarBox::m_SharedMemory = &qSharedMemory;
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  VarBox::GetInstance();
  VarBox::GetSpeedBox()->Show();
  a.exec();
  qSharedMemory.detach();
  return 0;
}
