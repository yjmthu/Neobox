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
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>

using namespace std::literals;
namespace fs = std::filesystem;

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

class Installer: public QWidget
{
public:
  explicit Installer(fs::path desDir)
    : QWidget()
    , m_DestinationPath(std::move(desDir))
    , m_DesktopShortcut(new QCheckBox("桌面快捷方式", this))
    , m_StartMenuShortcut(new QCheckBox("开始菜单快捷方式", this))
    , m_RunApplication(new QCheckBox("运行程序"))
    , m_AutoStart(new QCheckBox("开启自启"))
  {
    setWindowTitle("Neobox 安装程序");
    auto const mainLayout = new QVBoxLayout(this);

    mainLayout->addWidget(new QLabel("选择安装之后的操作：", this));
    auto layout = new QHBoxLayout;
    mainLayout->addLayout(layout);
    layout->addWidget(m_DesktopShortcut);
    layout->addWidget(m_StartMenuShortcut);
    layout->addWidget(m_AutoStart);
    layout->addWidget(m_RunApplication);
    m_DesktopShortcut->setChecked(true);
    m_StartMenuShortcut->setChecked(true);
    m_RunApplication->setChecked(true);
    m_AutoStart->setChecked(true);

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
      LinkTo(pathDesktopLink, L"neobox.exe");
    }
    if (m_StartMenuShortcut->isChecked()) {
      auto strStartMenu = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
      fs::path pathStartMenu = strStartMenu.toStdWString();
      pathStartMenu /= "Neobox";
      if (!fs::exists(pathStartMenu)) {
        fs::create_directory(pathStartMenu);
      }
      LinkTo(pathStartMenu / "Neobox.lnk", L"neobox.exe");
      LinkTo(pathStartMenu / "Unistaller.lnk", L"uninstaller.exe");
    }
    const auto mainExePath = QString::fromStdWString((m_DestinationPath / "neobox.exe").wstring());

    if (m_AutoStart->isChecked()) {
      QSettings settings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)", QSettings::NativeFormat);
      settings.setValue("Neobox", QStringLiteral("\"%1\"").arg(mainExePath));
    }

    if (m_RunApplication->isChecked()) {
      QProcess::startDetached(mainExePath, { "new" });
    }
    QApplication::quit();
  }

  void LinkTo(fs::path path, std::wstring exeName) const {
    if (fs::exists(path)) {
      fs::remove(path);
    }
    QFile::link(
      QString::fromStdWString((m_DestinationPath / exeName).wstring()),
      QString::fromStdWString(path.wstring())
    );
  }

private:
  const fs::path m_DestinationPath;
  QCheckBox *m_DesktopShortcut;
  QCheckBox *m_StartMenuShortcut;
  QCheckBox *m_RunApplication;
  QCheckBox *m_AutoStart;
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

class ChooseDirDlg: public QDialog
{
public:
  explicit ChooseDirDlg()
    : QDialog()
    , m_Admin(new QRadioButton("所有用户", this))
    , m_User(new QRadioButton("当前用户", this))
    , m_Other(new QRadioButton("其它位置", this))
    , m_DirPath(new QLineEdit(this))
    , m_Button(new QPushButton("确定", this))
  {
    setWindowTitle("Neobox安装程序");
    auto group = QButtonGroup(this);
    group.addButton(m_Admin);
    group.addButton(m_User);
    group.addButton(m_Other);

    auto mainLayout = new QVBoxLayout(this);
    auto layout = new QHBoxLayout();
    mainLayout->addLayout(layout);
    layout->addWidget(m_Admin);
    layout->addWidget(m_User);
    layout->addWidget(m_Other);

    mainLayout->addWidget(m_DirPath);
    m_DirPath->setReadOnly(true);

    layout = new QHBoxLayout();
    mainLayout->addLayout(layout);
    layout->addStretch();
    layout->addWidget(m_Button);

    connect(m_Admin, &QRadioButton::clicked, this, [this](){
      m_DirPath->setText(QString::fromStdWString(m_ProgramFiles.wstring()));
    });
    connect(m_User, &QRadioButton::clicked, this, [this](){
      m_DirPath->setText(QString::fromStdWString(m_LocalAppData.wstring()));
    });
    connect(m_Other, &QRadioButton::clicked, this, [this]() {
      auto curDir = QString::fromStdWString(m_ProgramFiles.parent_path().wstring());
      auto dirString = QFileDialog::getExistingDirectory(this, "选择安装位置", curDir);
      if (!dirString.isEmpty()) {
        fs::path userDir = dirString.toStdWString();
        auto name = userDir.filename().u8string();
        for (auto& i: name) {
          if (std::islower(i)) {
            i = std::toupper(i);
          }
        }
        if (name != u8"NEOBOX") {
          userDir /= "Neobox";
        }
        userDir.make_preferred();
        m_DirPath->setText(QString::fromStdWString(userDir.wstring()));
      }
    });
    connect(m_Button, &QPushButton::clicked, this, [this]() {
      m_DirPathString = m_DirPath->text();
      close();
    });
  
    m_Admin->click();
  }
  virtual ~ChooseDirDlg() {}

  static std::optional<fs::path> GetPath() {
    ChooseDirDlg dlg;
    dlg.exec();
    if (dlg.m_DirPathString.isEmpty()) {
      return std::nullopt;
    }
    fs::path result = dlg.m_DirPathString.toStdWString();
    result.make_preferred();
    if (!fs::exists(result)) {
      fs::create_directories(result);
    }

    return result;
  }
private:
  QRadioButton* m_Admin;
  QRadioButton* m_User;
  QRadioButton* m_Other;
  QLineEdit* const m_DirPath;
  QPushButton* m_Button;
  
  QString m_DirPathString;

  inline static const std::wstring m_DirectorayName = L"Neobox";
  const fs::path m_LocalAppData = GetPath(CSIDL_LOCAL_APPDATA) / "Programs" / m_DirectorayName;
  const fs::path m_ProgramFiles = GetPath(CSIDL_PROGRAM_FILES) / m_DirectorayName;

  static fs::path GetPath(int type) {
    std::wstring result(MAX_PATH, L'\0');
    SHGetSpecialFolderPathW(0, result.data(), type, TRUE);
    result.erase(result.find(L'\0'));
    return result;
  }
};

int main(int argc, char*argv[]) {
  QApplication app(argc, argv);

  fs::path desDir;
  if (argc < 2) {
    auto dir = ChooseDirDlg::GetPath();
    if (!dir) return 0;
    desDir = std::move(*dir);
  } else {
    std::this_thread::sleep_for(1s);
    desDir = argv[1];
  }
  desDir.make_preferred();

  if (CheckInstance()) {
    while (true) {
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
        break;
      } catch (fs::filesystem_error err) {
        if (err.code().value() == 32) {
          if (QMessageBox::question(nullptr, "提示", "程序可能被占用，是否重试？") == QMessageBox::Ok) {
            continue;
          }
          return 0;
        }
        auto message = std::format(
          L"文件操作出错！\n错误描述：{}\n错误信息：{}\n错误码：{}。",
          Ansi2WideString(err.what()),
          Ansi2WideString(err.code().message()),
          err.code().value()
        );
        QMessageBox::critical(nullptr, "出错", QString::fromStdWString(message));
        return 0;
      }
    }
  } else {
    return 0;
  }

  if (argc == 1) {
    Installer i(desDir);
    i.show();
    app.exec();
    WriteRegister(desDir);
  }
  return 0;
}
