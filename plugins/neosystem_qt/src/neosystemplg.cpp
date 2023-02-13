#include <neosystemplg.h>
#include <yjson.h>
#include <systemapi.h>
#include <menubase.hpp>

#include <QDir>

#ifdef _WIN32
#include <windows.h>
#include <powrprof.h>
#endif

#define CLASS_NAME NeoSystemPlg
#include <pluginexport.cpp>


NeoSystemPlg::NeoSystemPlg(YJson& settings):
  PluginObject(InitSettings(settings), u8"neosystemplg", u8"系统控制")
{
  LoadResources();
  InitFunctionMap();
}

NeoSystemPlg::~NeoSystemPlg()
{
#ifdef _WIN32
  if (m_Settings[u8"StopSleep"].isTrue()) {
    SetThreadExecutionState(ES_CONTINUOUS);
  }
#endif
  delete m_MainMenuAction;
}

void NeoSystemPlg::InitFunctionMap() {
  m_PluginMethod = {
    {u8"shutdownComputer",
      {u8"快速关机", u8"关闭计算机", [](PluginEvent, void*) {
#ifdef _WIN32
        ShellExecuteW(nullptr, L"open", L"shutdown", L"-s -t 0", nullptr, 0);
#else
        system("systemctl poweroff");
#endif
      }, PluginEvent::Void},
    },
    {u8"restartComputer",
      {u8"快捷重启", u8"重启计算机", [](PluginEvent, void*){
#ifdef _WIN32
        ShellExecuteW(nullptr, L"open", L"shutdown", L"-r -t 0", nullptr, 0);
#else
        system("systemctl reboot");
#endif
      }, PluginEvent::Void},
    },
    {u8"suspendedComputer",
      {u8"进入睡眠", u8"计算机进入睡眠模式", [](PluginEvent, void*){
#ifdef _WIN32
        SetSuspendState(FALSE, TRUE, FALSE);
#else
        system("systemctl suspend");
#endif
      }, PluginEvent::Void},
    },
    {u8"hibernateComputer",
      {u8"开启休眠", u8"计算机进入休眠模式", [](PluginEvent, void*){
#ifdef _WIN32
        SetSuspendState(TRUE, TRUE, TRUE);
#else
        system("systemctl hibernate");
#endif
      }, PluginEvent::Void},
    },

#ifdef _WIN32
    {u8"enableCopyPath",
      {u8"复制路径", u8"在文件资源管理器右键菜单添加“复制路径选项”", [](PluginEvent event, void* data) {
        if (event == PluginEvent::Bool) {
          SetDesktopRightMenu(*reinterpret_cast<bool *>(data));
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool *>(data) = HasDesktopRightMenu();
        }
      }, PluginEvent::Bool},
    },
    {u8"enableStopSleep",
      {u8"防止息屏", u8"防止电脑自动锁屏或休眠", [this](PluginEvent event, void* data){
        if (event == PluginEvent::Bool) {
          auto const on = *reinterpret_cast<bool *>(data);
          if (on) {
            SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
            mgr->ShowMsg("开启成功");
          } else {
            SetThreadExecutionState(ES_CONTINUOUS);
            mgr->ShowMsg("关闭成功");
          }
          m_Settings[u8"StopSleep"] = on;
          mgr->SaveSettings();
        } else if (event == PluginEvent::BoolGet) {
          auto& on = *reinterpret_cast<bool *>(data);
          on = m_Settings[u8"StopSleep"].isTrue();
          if (on) {
            SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
          }
        }
      }, PluginEvent::Bool}
    },
#endif
  };
}

QAction* NeoSystemPlg::LoadMainMenuAction()
{
  auto names = {u8"shutdownComputer", u8"restartComputer", u8"suspendedComputer"};

  m_MainMenuAction = new QAction("系统控制");
  auto const menu = new MenuBase(m_MainMenu);
  m_MainMenuAction->setMenu(menu);

  auto actions = m_MainMenu->actions();
  auto iter = std::find_if(actions.begin(), actions.end(), [](QAction* action){ return action->text() == "防止息屏"; });
  if (iter != actions.end()) menu->addAction(*iter);

  for (const auto& name: names) {
    const auto& info = m_PluginMethod[name];
    auto const action = menu->addAction(
          Utf82QString(info.friendlyName));
    action->setToolTip(PluginObject::Utf82QString(info.description));
    QObject::connect(action, &QAction::triggered, menu, std::bind(info.function, PluginEvent::Void, nullptr));
    m_PluginMethod[name] = std::move(info);
  }

  return m_MainMenuAction;
}

QAction* NeoSystemPlg::InitMenuAction()
{
  this->PluginObject::InitMenuAction();
  return LoadMainMenuAction();
}

YJson& NeoSystemPlg::InitSettings(YJson& settings)
{
  if (settings.isObject()) return settings;
  return settings = YJson::O {
    { u8"StopSleep", false },
  };
  // we may not need to call SaveSettings;
}

#ifdef _WIN32
void NeoSystemPlg::SetDesktopRightMenu(bool on)
{
  constexpr auto prefix = L"Software\\Classes\\{}\\shell";
  const std::initializer_list<std::pair<std::wstring, wchar_t>> lst = {
      {L"*", L'1'},
      {L"Directory", L'V'},
      {L"Directory\\Background", L'V'}};
  constexpr auto command =
      L"mshta vbscript:clipboarddata.setdata(\"text\",\"%{}\")(close)";
  if (on) {
    for (const auto& [param1, param2] : lst) {
      HKEY hKey = nullptr;
      std::wstring wsSubKey = std::format(prefix, param1) + L"\\Neobox";
      std::wstring cmdstr = std::format(command, param2);
      std::wstring wsIconFileName =
          std::filesystem::absolute("icons/copy.ico").wstring();
      if (RegCreateKeyW(HKEY_CURRENT_USER, wsSubKey.c_str(), &hKey) !=
          ERROR_SUCCESS) {
        return;
      }
      RegSetValueW(hKey, L"command", REG_SZ, cmdstr.c_str(), 0);
      RegSetValueW(hKey, nullptr, REG_SZ, L"复制路径", 0);
      RegSetValueExW(
          hKey, L"Icon", 0, REG_SZ,
          reinterpret_cast<const BYTE*>(wsIconFileName.data()),
          (DWORD)wsIconFileName.size() * sizeof(wchar_t));
      RegCloseKey(hKey);
    }
  } else {
    for (const auto& [param1, param2] : lst) {
      HKEY hKey = nullptr;
      std::wstring wsSubKey = std::format(prefix, param1);
      if (RegOpenKeyExW(HKEY_CURRENT_USER, wsSubKey.c_str(), 0,
                        KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        continue;
      }
      RegDeleteTreeW(hKey, L"Neobox");
      RegCloseKey(hKey);
    }
  }
}

bool NeoSystemPlg::HasDesktopRightMenu()
{
  constexpr auto prefix = L"Software\\Classes\\{}\\shell";
  const auto lst = {L"*", L"Directory", L"Directory\\Background"};
  for (const auto& i : lst) {
    std::wstring wsSubKey = std::format(prefix, i);
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, wsSubKey.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
      return false;
    }
    RegCloseKey(hKey);
  }
  return true;
}
#endif

void NeoSystemPlg::LoadResources()
{
  QDir dir;
  if (!dir.exists("icons"))
    dir.mkdir("icons");
  QString const des = "icons/copy.ico";
  QFile::copy(":/" + des, des);
  QFile::setPermissions(des, QFile::ReadUser | QFile::WriteUser);
}
