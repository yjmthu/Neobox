#include <neomenu.h>
#include <pluginmgr.h>
#include <systemapi.h>
#include <versiondlg.h>
#include <neoapp.h>

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QWidget>
#include <QMessageBox>

#include <QClipboard>
#include <QMimeData>

#ifdef _WIN32
#include <Windows.h>
#elif def __linux__
#else
#endif

NeoMenu::NeoMenu(GlbObject* glb)
    : QMenu(nullptr),
    m_GlbObject(glb)
{
  setAttribute(Qt::WA_TranslucentBackground, true);
  setToolTipsVisible(true);
  InitStyleSheet();
  InitSettingMenu();
  InitPluginMenu();
}

NeoMenu::~NeoMenu() {
  delete m_PluginMgr;
}

void NeoMenu::InitStyleSheet()
{
  static const char szStylePath[] = ":/styles/AppStyle.css";
  if (QFile::exists(szStylePath)) {
    QFile file(szStylePath);
    if (file.open(QIODevice::ReadOnly)) {
      setStyleSheet(file.readAll());
      file.close();
      return;
    }
  }
  throw nullptr;
}

void NeoMenu::InitPluginMenu()
{
  m_PluginMenu = new QMenu(this);
  m_PluginMenu->setAttribute(Qt::WA_TranslucentBackground, true);
  m_PluginMenu->setToolTipsVisible(true);
  addAction("插件设置")->setMenu(m_PluginMenu);
}

void NeoMenu::InitPluginMgr()
{
  m_PluginMgr = new PluginMgr(m_GlbObject, m_PluginMenu);
  m_PluginMgr->LoadPlugins();
  m_PluginMgr->LoadManageAction(m_SettingMenu->addAction("插件管理"));
  InitFunctionMap();
}

void NeoMenu::InitSettingMenu()
{
  m_SettingMenu = new QMenu(this);
  m_SettingMenu->setAttribute(Qt::WA_TranslucentBackground, true);
  m_SettingMenu->setToolTipsVisible(true);
  addAction("设置中心")->setMenu(m_SettingMenu);
  auto const action = m_SettingMenu->addAction("开机自启");
  action->setCheckable(true);
  action->setChecked(IsAutoStart());
  connect(action, &QAction::triggered, this, std::bind(&NeoMenu::SetAutoSatrt, this, action, std::placeholders::_1));
  // m_SettingMenu->addSeparator();
}

void NeoMenu::InitFunctionMap() {
  connect(m_SettingMenu->addAction("程序位置"), &QAction::triggered, this, std::bind(QDesktopServices::openUrl,
                 QUrl::fromLocalFile(qApp->applicationDirPath())));
  connect(m_SettingMenu->addAction("配置目录"), &QAction::triggered, this, std::bind(QDesktopServices::openUrl,
                 QUrl::fromLocalFile(QDir::currentPath())));
  connect(m_SettingMenu->addAction("关于软件"), &QAction::triggered, this, [](){(new VersionDlg)->show();});
  connect(m_SettingMenu->addAction("退出软件"), &QAction::triggered, this, QApplication::quit);
  /*
  m_FuncCheckMap = {
      {u8"ToolTransRegistKey",
       {[this](bool checked) {
          auto& settings = VarBox::GetSettings(u8"Tools");
          std::u8string& shortcut =
              settings[u8"Translate.Shortcut"].getValueString();
          QString qsShortcut =
              QString::fromUtf8(shortcut.data(), shortcut.size());
          if (checked) {
            m_Shortcut->RegistHotKey(qsShortcut,
                                     m_FuncNormalMap[u8"ToolTransShowDlg"]);
          } else {
            m_Shortcut->UnregistHotKey(qsShortcut);
          }
          settings[u8"Translate.RegisterHotKey"] = checked;
          VarBox::WriteSettings();
        },
        [this]() -> bool {

        }}},
      };
  */
}

bool NeoMenu::IsAutoStart()
{
  /*
  auto& settings = VarBox::GetSettings(u8"Tools");
  bool regist = settings[u8"Translate.RegisterHotKey"].isTrue();
  std::u8string& shortcut =
      settings[u8"Translate.Shortcut"].getValueString();
  QString qsShortcut =
      QString::fromUtf8(shortcut.data(), shortcut.size());
  if (regist && !m_Shortcut->IsKeyRegisted(qsShortcut)) {
    m_Shortcut->RegistHotKey(qsShortcut,
                              m_FuncNormalMap[u8"ToolTransShowDlg"]);
  }
  return regist;*/
  wchar_t pPath[] =
      L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
  return GetExeFullPath() ==
          RegReadString(HKEY_CURRENT_USER, pPath, L"Neobox");
}

void NeoMenu::SetAutoSatrt(QAction* action, bool on)
{
  wchar_t pPath[] =
      L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
  wchar_t pAppName[] = L"Neobox";
  std::wstring wsThisPath = GetExeFullPath();
  std::wstring wsThatPath =
      RegReadString(HKEY_CURRENT_USER, pPath, pAppName);
  if (wsThatPath != wsThisPath) {
    if (on && !RegWriteString(HKEY_CURRENT_USER, pPath, pAppName, wsThisPath)) {
      action->setChecked(false);
      m_GlbObject->glbShowMsg("设置自动启动失败！");
    } else {
      m_GlbObject->glbShowMsg("设置成功！");
    }
  } else {
    if (!on && !RegRemoveValue(HKEY_CURRENT_USER, pPath, pAppName)) {
      action->setChecked(true);
      m_GlbObject->glbShowMsg("取消自动启动失败！");
    } else {
      m_GlbObject->glbShowMsg("设置成功！");
    }
  }
}
