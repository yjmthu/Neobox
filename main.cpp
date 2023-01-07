#include <QApplication>
#include <glbobject.h>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  try {
    GlbObject glbObject;
    glbObject.Exec();
  } catch (std::runtime_error err) {
    //
  } catch (...) {
    //
  }
  return 0;
}

