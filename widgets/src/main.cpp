#include <QApplication>
#include <QProcess>

#include <speedbox.h>
#include <varbox.h>
#include <appcode.hpp>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  a.setQuitOnLastWindowClosed(false);
  VarBox var;
  SpeedBox* box = VarBox::GetSpeedBox();
  box->Show();
  if (a.exec() == (int)ExitCode::RETCODE_RESTART) {
    QProcess::startDetached(a.applicationFilePath(), QStringList());
  }
  return 0;
}
