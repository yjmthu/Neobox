#include <filesystem>
#include <thread>
#include <chrono>
#include <format>
#include <systemapi.h>

#include <QMessageBox>
#include <QSharedMemory>
#include <QFileDialog>
#include <QProcess>
#include <QApplication>

using namespace std::literals;
namespace fs = std::filesystem;

void CopyDir(fs::path res, fs::path des) {

  if (!fs::exists(des)) {
    fs::create_directory(des);
  }

  for (const auto& entry: fs::directory_iterator(res)) {
    const auto currentPath = entry.path();
    const auto newPath = des / currentPath.filename();

    if (fs::is_directory(currentPath)) {
      CopyDir(currentPath, newPath);
    } else if (fs::is_regular_file(currentPath)) {
      fs::copy(currentPath, newPath, fs::copy_options::overwrite_existing);
    }
  }
}

bool CheckInstance() {
  QSharedMemory memory("__Neobox__");
  while (memory.attach(QSharedMemory::ReadOnly)) {
    memory.detach();
    auto res = QMessageBox::question(nullptr, "提示", "请先退出Neobox后重试！");
    if (res == QMessageBox::No) {
      return false;
    }
  }
  return true;
}

int main(int argc, char*argv[]) {
  QApplication app(argc, argv);

  fs::path desDir;
  if (argc < 2) {
    auto dir = QFileDialog::getExistingDirectory(nullptr, "选择安装位置");
    if (dir.isEmpty()) return 0;
    desDir = dir.toStdWString();
  } else {
    std::this_thread::sleep_for(1s);
    desDir = argv[1];
  }
  desDir.make_preferred();

  if (CheckInstance()) {
    try {
      fs::path resDir = fs::absolute(argv[0]).parent_path();
      fs::current_path(desDir.parent_path());
      CopyDir(resDir, desDir);

      fs::path exeFilePath = desDir / "neobox.exe";
      auto exeFile = exeFilePath.wstring();
      exeFile.push_back(L'\0');
      QProcess::startDetached(QString::fromStdWString(exeFile), { "new" });
    } catch (fs::filesystem_error err) {
      auto message = std::format(
        L"文件操作出错！\n错误描述：{}\n错误信息：{}\n错误码：{}。",
        Ansi2WideString(err.what()),
        Ansi2WideString(err.code().message()),
        err.code().value()
      );
      QMessageBox::critical(nullptr, "出错", QString::fromStdWString(message));
    }
  }

  return 0;
}
