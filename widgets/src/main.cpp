#include <QApplication>

#include <speedbox.h>
#include <varbox.h>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  VarBox var;
  SpeedBox box;
  box.show();
  a.exec();
  return 0;
}
