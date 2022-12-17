#include <neohotkeyplg.h>
#include <shortcutdlg.h>
#include <shortcut.h>

#include <windows.h>
#include <QApplication>

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
    auto name = m_Shortcut->GetPluginName(msg->wParam);
    SendBroadcast(PluginEvent::HotKey, &name);
  }
  return false;
}

void NeoHotKeyPlg::InitFunctionMap()
{
  m_PluginMethod = {
    {u8"showDialog",
      {u8"控制面版", u8"注册/取消热键或生成新的热键。", [this](PluginEvent, void*) {
        auto const dlg = new ShortcutDlg(m_Settings[u8"HotKeys"]);
        dlg->show();
        QObject::connect(dlg, &ShortcutDlg::finished, dlg, [this](){
          m_Shortcut->UnregisterAllHotKey();
          m_Shortcut->RegisterAllHotKey();
        });
      }, PluginEvent::Void},
    },
  };
}

QAction* NeoHotKeyPlg::InitMenuAction()
{
  return this->PluginObject::InitMenuAction();
}

YJson& NeoHotKeyPlg::InitSettings(YJson& settings)
{
  if (settings.isObject()) {
    return settings;
  }
  return settings = YJson::O {
    {u8"HotKeys", YJson::A {
      YJson::O {
        {u8"KeySequence", u8"Shift+Z"},
        {u8"Enabled", false},
        {u8"Plugin", u8"neotranslateplg"},
      },
      YJson::O {
        {u8"KeySequence", u8"Ctrl+Shift+A"},
        {u8"Enabled", false},
        {u8"Plugin", u8"neoocrplg"},
      },
    }},
  };
}
