#include <neohotkeyplg.h>
#include <shortcut.h>

#include <windows.h>
#include <QApplication>

#define CLASS_NAME NeoHotKeyPlg
#include <pluginexport.cpp>

NeoHotKeyPlg::NeoHotKeyPlg(YJson& settings):
  PluginObject(InitSettings(settings), u8"neohotkeyplg", u8"热键注册"),
  m_Shortcut(new Shortcut)
{
  InitFunctionMap();
  qApp->installNativeEventFilter(this);
}

NeoHotKeyPlg::~NeoHotKeyPlg()
{
  delete m_Shortcut;
  // for (auto& [_, j]: m_Settings[u8"HotKeys"].getObject()) {
    // UnregisterHotKey(NULL, 1234);
  // }
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
    for (const auto& fun: m_Followers) {
      fun->operator()(PluginEvent::HotKey, message);
    }
  }
  return false;
}

void NeoHotKeyPlg::InitFunctionMap()
{
  m_FunctionMapVoid = {
    {u8"showDialog",
      {u8"控制面版", u8"注册/取消热键或生成新的热键。", [this]() {
      }},
    },
  };
}

void NeoHotKeyPlg::InitMenuAction()
{
  this->PluginObject::InitMenuAction();
}

YJson& NeoHotKeyPlg::InitSettings(YJson& settings)
{
  if (settings.isObject()) {
    return settings;
  }
  return settings = YJson::O {
    {u8"HotKeys", YJson::O {
      {u8"开关翻译窗口", YJson::O {
        {u8"KeySequence", YJson::A { "Shift", "Z" }},
        {u8"Enabled", false},
      }},
      {u8"屏幕截图", YJson::O {
        {u8"KeySequence", YJson::A { "Ctrl", "Shift", "A" }},
        {u8"Enabled", false},
      }},
    }},
  };
}
