#include <neohotkeyplg.h>
#include <shortcut.h>
#include <systemapi.h>

#include "..\widgets\shortcutdlg.hpp"

#include <windows.h>
#include <QApplication>

#include <filesystem>

namespace fs = std::filesystem;

#define CLASS_NAME NeoHotKeyPlg
#include <pluginexport.cpp>

NeoHotKeyPlg::NeoHotKeyPlg(YJson& settings):
  PluginObject(InitSettings(settings), u8"neohotkeyplg", u8"热键注册"),
  m_Shortcut(new Shortcut(settings[u8"HotKeys"]))
{
  InitFunctionMap();
  qApp->installNativeEventFilter(this);
}

NeoHotKeyPlg::~NeoHotKeyPlg()
{
  delete m_Shortcut;
}

bool NeoHotKeyPlg::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
  if(eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG")
    return false;
  MSG* msg = static_cast<MSG*>(message);

  if (WM_HOTKEY == msg->message) {
    /*
     * idHotKey = wParam;
     * Modifiers = (UINT) LOWORD(lParam);
     * uVirtKey = (UINT) HIWORD(lParam);
     */
    auto& info = m_Shortcut->GetCallbackInfo(msg->wParam);
    
    if (info.type == Shortcut::Command) {
      auto& data = m_Settings[u8"Commands"][info.name];
      fs::path executable = data[u8"Executable"].getValueString();
      executable.make_preferred();
      auto& arguments = data[u8"Arguments"].getValueString();
      fs::path directory = data[u8"Directory"].getValueString();
      directory.make_preferred();

      auto exe = executable.wstring();
      auto arg = Utf82WideString(arguments);
      auto dir = directory.wstring();

      ShellExecute(nullptr, L"open", exe.c_str(), arg.c_str(),
        dir.c_str(), SW_SHOWNORMAL);
      
    } else if (info.type == Shortcut::Plugin) {
      auto name = info.name;
      SendBroadcast(PluginEvent::HotKey, &name);
    }
  }
  return false;
}

void NeoHotKeyPlg::InitFunctionMap()
{
  m_PluginMethod = {
    {u8"showDialog",
      {u8"控制面版", u8"注册/取消热键或生成新的热键。", [this](PluginEvent event, void* data) {
        if (event == PluginEvent::HotKey && 
          *reinterpret_cast<std::u8string*>(data) != u8"neohotkeyplg")
          return;
        auto const dlg = new ShortcutDlg(m_Settings[u8"HotKeys"]);
        dlg->show();
        QObject::connect(dlg, &ShortcutDlg::finished, dlg, [this](){
          m_Shortcut->UnregisterAllHotKey();
          m_Shortcut->RegisterAllHotKey();
        });
      }, PluginEvent::Void},
    },
  };

  m_Followers.insert(&m_PluginMethod[u8"showDialog"].function);
}

QAction* NeoHotKeyPlg::InitMenuAction()
{
  return this->PluginObject::InitMenuAction();
}

YJson& NeoHotKeyPlg::InitSettings(YJson& settings)
{
  if (!settings.isObject()) {
    settings = YJson::O {
      {u8"HotKeys", YJson::A {
        YJson::O {
          {u8"KeySequence", u8"Shift+Z"},
          {u8"Enabled", false},
          {u8"Plugin", u8"neotranslateplg"},
        },
        YJson::O {
          {u8"KeySequence", u8"Ctrl+Shift+A"},
          {u8"Enabled", false},
          {u8"Command", u8"打开文件资源管理器"},
        },
      }},
    };
  }
  auto& version = settings[u8"Version"];
  if (!version.isNumber()) {
    version = 0;
    settings[u8"Commands"] = YJson::O {
      {u8"打开文件资源管理器", YJson::O {
        {u8"Executable", u8"explorer.exe"},
        {u8"Directory", u8"."},
        {u8"Arguments", YJson::String}
      }},
      {u8"10秒后关机", YJson::O {
        {u8"Executable", u8"shutdown.exe"},
        {u8"Directory", u8"."},
        {u8"Arguments", u8"-s -t 10"}
      }},
    };
  }
  return settings;
}
