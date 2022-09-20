#include <QApplication>
#include <QProcess>

#include <appcode.hpp>
#include <speedbox.h>
#include <varbox.h>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  VarBox var;
  VarBox::GetSpeedBox()->Show();
  if (a.exec() == (int)ExitCode::RETCODE_RESTART) {
    QProcess::startDetached(a.applicationFilePath(), QStringList());
  }
  return 0;
}

