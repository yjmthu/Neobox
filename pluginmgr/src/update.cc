#include <filesystem>
#include <thread>
#include <chrono>
#include <format>
#include <string>

#include <systemapi.h>
#include <config.h>

#include <windows.h>
#include <shlobj.h>
#include <objbase.h>

#include <QMessageBox>
#include <QSharedMemory>
#include <QFileDialog>
#include <QProcess>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QSettings>

using namespace std::literals;
namespace fs = std::filesystem;

class Installer: public QWidget
{
public:
  explicit Installer(fs::path desDir)
    : QWidget()
    , m_DestinationPath(std::move(desDir /= "neobox.exe"))
    , m_DesktopShortcut(new QCheckBox("桌面快捷方式", this))
    , m_StartMenuShortcut(new QCheckBox("开始菜单快捷方式", this))
    , m_RunApplication(new QCheckBox("运行程序"))
  {
    setWindowTitle("Neobox 安装程序");
    auto const mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(new QLabel("选择安装之后的操作：", this));
    auto layout = new QHBoxLayout;
    mainLayout->addLayout(layout);
    layout->addWidget(m_DesktopShortcut);
    layout->addWidget(m_StartMenuShortcut);
    layout->addWidget(m_RunApplication);
    m_DesktopShortcut->setChecked(true);
    m_StartMenuShortcut->setChecked(true);
    m_RunApplication->setChecked(true);

    layout = new QHBoxLayout;
    mainLayout->addLayout(layout);
    layout->addStretch();
    auto const button = new QPushButton("确定", this);
    layout->addWidget(button);
    connect(button, &QPushButton::clicked, this, &Installer::CopyShortcuts);
  }
  virtual ~Installer() {}
private:
  void CopyShortcuts() const {
    if (m_DesktopShortcut->isChecked()) {
      auto strDesktopLink = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
      fs::path pathDesktopLink = strDesktopLink.toStdWString();
      pathDesktopLink /= "Neobox.lnk";
      LinkTo(pathDesktopLink);
    }
    if (m_StartMenuShortcut->isChecked()) {
      auto strStartMenu = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
      fs::path pathStartMenu = strStartMenu.toStdWString();
      pathStartMenu /= "Neobox";
      if (!fs::exists(pathStartMenu)) {
        fs::create_directory(pathStartMenu);
      }
      pathStartMenu /= "Neobox.lnk";
      LinkTo(pathStartMenu);
    }
    if (m_RunApplication->isChecked()) {
      QProcess::startDetached(QString::fromStdWString(m_DestinationPath.wstring()), { "new" });
    }
    QApplication::quit();
  }

  void LinkTo(fs::path path) const {
    if (fs::exists(path)) {
      fs::remove(path);
    }
    QFile::link(
      QString::fromStdWString(m_DestinationPath.wstring()),
      QString::fromStdWString(path.wstring())
    );
  }

private:
  const fs::path m_DestinationPath;
  QCheckBox *m_DesktopShortcut;
  QCheckBox *m_StartMenuShortcut;
  QCheckBox *m_RunApplication;
};

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

void WriteRegister(fs::path installDir) {
  // 创建QSettings对象，指定注册表的路径和组织名称
  QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Neobox", QSettings::NativeFormat);

  auto version = u8"" NEOBOX_VERSION ""sv.substr(1);
  auto app = installDir / "neobox.exe";
  auto uninstall = installDir / "uninstaller.exe";
  // 写入注册表值
  settings.setValue("DisplayName", "Neobox");
  settings.setValue("Publisher", "yjmthu");
  settings.setValue("DisplayVersion", QString::fromUtf8(version.data(), version.size()));
  settings.setValue("DisplayIcon", QString::fromStdWString(app.wstring()));
  settings.setValue("UninstallString", QString::fromStdWString(uninstall.wstring()));
  settings.setValue("InstallLocation", QString::fromStdWString(installDir.wstring()));
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
      resDir.make_preferred();
      if (resDir != desDir)
        CopyDir(resDir, desDir);
      // fs::current_path(desDir.parent_path());

      fs::path exeFilePath = desDir / "neobox.exe";
      auto exeFile = exeFilePath.wstring();
      exeFile.push_back(L'\0');
      if (argc > 1)
        QProcess::startDetached(QString::fromStdWString(exeFile), { "new" });
    } catch (fs::filesystem_error err) {
      auto message = std::format(
        L"文件操作出错！\n错误描述：{}\n错误信息：{}\n错误码：{}。",
        Ansi2WideString(err.what()),
        Ansi2WideString(err.code().message()),
        err.code().value()
      );
      QMessageBox::critical(nullptr, "出错", QString::fromStdWString(message));
      return 0;
    }
  } else {
    return 0;
  }

  if (argc == 1) {
    Installer i(desDir);
    i.show();
    app.exec();
  }
  WriteRegister(desDir);

  return 0;
}
