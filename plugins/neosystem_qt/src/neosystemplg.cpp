#include <neosystemplg.h>
#include <yjson.h>
#include <systemapi.h>

#include <QMenu>
#include <QDir>

#include <windows.h>

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
  //
}

void NeoSystemPlg::InitFunctionMap() {
  m_PluginMethod = {
    {u8"shutdownComputer",
      {u8"快速关机", u8"关闭计算机", [](PluginEvent, void*) {
        ShellExecuteW(nullptr, L"open", L"shutdown", L"-s -t 0", nullptr, 0);
      }, PluginEvent::Void},
    },
    {u8"restartComputer",
      {u8"快捷重启", u8"重启计算机", [](PluginEvent, void*){
        ShellExecuteW(nullptr, L"open", L"shutdown", L"-r -t 0", nullptr, 0);
      }, PluginEvent::Void},
    },

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
          SetThreadExecutionState(on ?
              (ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED) :
              ES_CONTINUOUS
          );
          m_Settings[u8"StopSleep"] = on;
          mgr->SaveSettings();
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool *>(data) = m_Settings[u8"StopSleep"].isTrue();
        }
      }, PluginEvent::Bool}
    },
  };
}

void NeoSystemPlg::InitMenuAction()
{
  this->PluginObject::InitMenuAction();
}

YJson& NeoSystemPlg::InitSettings(YJson& settings)
{
  if (settings.isObject()) return settings;
  return settings = YJson::O {
    { u8"StopSleep", false },
  };
  // we may not need to call SaveSettings;
}

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

void NeoSystemPlg::LoadResources()
{
  QDir dir;
  if (!dir.exists("icons"))
    dir.mkdir("icons");
  QString const des = "icons/copy.ico";
  QFile::copy(":/" + des, des);
  QFile::setPermissions(des, QFile::ReadUser | QFile::WriteUser);
}
