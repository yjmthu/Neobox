#include <neomenu.hpp>
#include <pluginmgr.h>
#include <systemapi.h>

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QWidget>
#include <QMessageBox>
#include <QProcess>

#include <QClipboard>
#include <QMimeData>

#ifdef _WIN32
#include <Windows.h>
#elif def __linux__
#else
#endif

NeoMenu::NeoMenu()
  : MenuBase(nullptr)
  , m_PluginMenu(new MenuBase(this))
  , m_ControlPanel(new QAction("控制面板"))
  , m_SettingMenu(new MenuBase(this))
{
  InitStyleSheet();
  InitSettingMenu();
  InitPluginMenu();
  InitFunctionMap();
}

NeoMenu::~NeoMenu() {
  delete m_PluginMenu;
  delete m_ControlPanel;
  delete m_SettingMenu;
}

void NeoMenu::InitStyleSheet()
{
  static const char szStylePath[] = ":/styles/AppStyle.qss";
  if (QFile::exists(szStylePath)) {
    QFile file(szStylePath);
    if (file.open(QIODevice::ReadOnly)) {
      setStyleSheet(file.readAll());
      file.close();
      return;
    }
  }
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
  connect(m_SettingMenu->addAction("程序位置"), &QAction::triggered, this, [](){
    std::wstring args = L"/select, " + GetExeFullPath();
    ShellExecuteW(nullptr, L"open", L"explorer", args.c_str(), NULL, SW_SHOWNORMAL);
  });
  connect(m_SettingMenu->addAction("配置目录"), &QAction::triggered, this, std::bind(QDesktopServices::openUrl,
                 QUrl::fromLocalFile(QDir::currentPath())));
  connect(m_SettingMenu->addAction("重启软件"), &QAction::triggered, this, std::bind(&PluginMgr::Restart, mgr));
  // connect(m_SettingMenu->addAction("关于软件"), &QAction::triggered, this, [](){(new VersionDlg)->show();});
  connect(m_SettingMenu->addAction("退出软件"), &QAction::triggered, this, QApplication::quit);
}

bool NeoMenu::IsAutoStart()
{
  wchar_t pPath[] = LR"(SOFTWARE\Microsoft\Windows\CurrentVersion\Run)";
  return std::format(L"\"{}\"", GetExeFullPath()) ==
          RegReadString(HKEY_CURRENT_USER, pPath, L"Neobox");
}

void NeoMenu::SetAutoSatrt(QAction* action, bool on)
{
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
}
