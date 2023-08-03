#include <config.h>

#include <filesystem>

#include <QApplication>
#include <QMessageBox>
#include <QSettings>

// namespace fs = std::filesystem;
using namespace std::literals;

void EraseRegister() {
  QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Neobox", QSettings::NativeFormat);
  settings.remove("");
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  if (QMessageBox::question(nullptr, "提示", "您真的要完全卸载Neobox吗？") == QMessageBox::No)
    return 0;
  EraseRegister();
  QMessageBox::information(nullptr, "提示", "卸载成功！");
  return 0;
}
