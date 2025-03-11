#include <neobox/unicode.h>
#include <neobox/neomenu.hpp>
#include <neobox/pluginmgr.h>
#include <neobox/process.h>
#include <neobox/widgetbase.hpp>

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QWidget>
#include <QMessageBox>
#include <QProcess>

#include <QMimeData>

#include <format>

#ifdef _WIN32
#include <Windows.h>
#include <neobox/systemapi.h>
using namespace std::literals;
#elif defined (__linux__)
#include <QStandardPaths>
#include <filesystem>
namespace fs = std::filesystem;
#endif

NeoMenu::NeoMenu()
  : MenuBase(nullptr)
  , m_SettingMenu(new MenuBase(this))
  , m_ControlPanel(new QAction("控制面板"))
  , m_PluginMenu(new MenuBase(this))
{
  InitSettingMenu();
  InitPluginMenu();
  InitFunctionMap();
}

NeoMenu::~NeoMenu() {
  delete m_PluginMenu;
  delete m_ControlPanel;
  delete m_SettingMenu;
}

void NeoMenu::InitPluginMenu()
{
  addAction("插件菜单")->setMenu(m_PluginMenu);
  addSeparator();
}

void NeoMenu::InitSettingMenu()
{
  addAction("设置中心")->setMenu(m_SettingMenu);
  m_SettingMenu->addAction(m_ControlPanel);
  m_SettingMenu->addSeparator();
  auto const action = m_SettingMenu->addAction("开机自启");
  action->setCheckable(true);
  action->setChecked(IsAutoStart());
  connect(action, &QAction::triggered, this, std::bind(&NeoMenu::SetAutoSatrt, this, action, std::placeholders::_1));
  // m_SettingMenu->addSeparator();
}

void NeoMenu::InitFunctionMap() {
#ifdef _WIN32
  connect(m_SettingMenu->addAction("程序位置"), &QAction::triggered, this, [](){
  std::filesystem::path exe = QApplication::applicationFilePath().toStdWString();
    std::wstring args = L"/select, " + exe.make_preferred().wstring();
    ShellExecuteW(nullptr, L"open", L"explorer", args.c_str(), NULL, SW_SHOWNORMAL);
  });
#else
  connect(m_SettingMenu->addAction("程序位置"), &QAction::triggered, this, std::bind(&QDesktopServices::openUrl, qApp->applicationDirPath()));
#endif
  connect(m_SettingMenu->addAction("配置目录"), &QAction::triggered, this, std::bind(QDesktopServices::openUrl,
                 QUrl::fromLocalFile(QDir::currentPath())));
  // std::ref可随mgr更新
  connect(m_SettingMenu->addAction("重启软件"), &QAction::triggered, this, &PluginMgr::Restart);
  // connect(m_SettingMenu->addAction("关于软件"), &QAction::triggered, this, [](){(new VersionDlg)->show();});
  connect(m_SettingMenu->addAction("退出软件"), &QAction::triggered, this, QApplication::quit);
}

#ifdef __linux__
static QString GetStartCommand() {
  return QStringLiteral("(sleep 10 && \"%1\") &").arg(
    QDir::toNativeSeparators(qApp->applicationFilePath()));
}

static inline fs::path GetHomePath() {
  return QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toStdWString();
}

static auto GetProfilePath() {
  auto home { GetHomePath() };
  auto cshell = std::getenv("SHELL");
  fs::path profile = ".profile";
  if (cshell) {
    fs::path shell = cshell;
    shell = shell.filename();
    if (shell == "zsh") {
      profile = ".zprofile";
    } else if (shell == "bash") {
      profile = ".bash_profile";
    }
  }
  return (home / profile).string();
}

#endif

bool NeoMenu::IsAutoStart()
{
#ifdef _WIN32
  wchar_t pPath[] = LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Run)";
  return std::format(L"\"{}\"", GetExeFullPath()) ==
          RegReadString(HKEY_CURRENT_USER, pPath, L"Neobox");
#else
  auto const profile = GetProfilePath();
  if (fs::exists(profile)) {
    auto start = GetStartCommand().toStdString();
    auto cmd = std::format("grep '{}' '{}'", start, profile);
    std::vector<std::string> result;
    // GetCmdOutput(cmd.c_str(), result);
    auto process = NeoProcess(AnsiAsUtf8(cmd));
    auto code = process.Run().get();
    if (!code || code.value() != 0) {
      return false;
    }
    auto output = process.GetStdOut();
    for (auto i = output.begin(); i != output.end(); ) {
      auto j = std::find(i, output.end(), '\n');

      if (std::equal(start.begin(), start.end(), i, j)) {
        auto profile = QString::fromUtf8(GetProfilePath());
        QProcess::execute("sed", {
          "-i", QStringLiteral("\\#^%1$#d").arg(QString::fromStdString(start)), profile
        });
        break;
      }

      i = j;
      if (i != output.end()) ++i;
    }
  }
  auto autostart { GetHomePath() / ".config/autostart/neobox.desktop" };
  return fs::exists(autostart);
#endif
}

void NeoMenu::SetAutoSatrt(QAction* action, bool on)
{
#ifdef _WIN32
  wchar_t pPath[] = LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Run)";
  wchar_t pAppName[] = L"Neobox";
  std::wstring wsThisPath = std::format(L"\"{}\"", GetExeFullPath());
  std::wstring wsThatPath =
      RegReadString(HKEY_CURRENT_USER, pPath, pAppName);
  if (wsThatPath != wsThisPath) {
    if (on && !RegWriteString(HKEY_CURRENT_USER, pPath, pAppName, wsThisPath)) {
      action->setChecked(false);
      mgr->ShowMsg("设置自动启动失败！");
    } else {
      mgr->ShowMsg("设置成功！");
    }
  } else {
    if (!on && !RegRemoveValue(HKEY_CURRENT_USER, pPath, pAppName)) {
      action->setChecked(true);
      mgr->ShowMsg("取消自动启动失败！");
    } else {
      mgr->ShowMsg("设置成功！");
    }
  }
#else
  bool succeed = false;
  std::error_code code;
  fs::path shortcut { "/usr/share/applications/neobox.desktop" };
  auto autostart { GetHomePath() / ".config/autostart" };
  if (!on) {
    autostart /= "neobox.desktop";
    if (fs::exists(autostart)) {
      succeed = fs::remove(autostart, code);
    } else {
      succeed = true;
    }
  } else if (fs::exists(shortcut)) {
    if (fs::exists(autostart) || fs::create_directories(autostart, code)) {
      autostart /= "neobox.desktop";
      succeed = !QProcess::execute("ln", {
        "-s", QString::fromStdWString(shortcut.wstring()), QString::fromStdWString(autostart.wstring())
      });
    }
  } else {
    succeed = false;
  }
  if (succeed) {
    mgr->ShowMsg("设置成功！");
  } else {
    action->setChecked(!on);
    mgr->ShowMsg("设置失败！");
  }
#endif
}
