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
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  VarBox var;
  VarBox::GetSpeedBox()->Show();
  auto iExitCode = static_cast<ExitCode>(a.exec());
    qSharedMemory.detach();
  if (iExitCode == ExitCode::RETCODE_RESTART) {
    QProcess::startDetached(a.applicationFilePath(), QStringList());
  }
  return 0;
}
