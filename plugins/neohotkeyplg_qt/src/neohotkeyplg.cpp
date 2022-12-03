#include <neohotkeyplg.h>

#define CLASS_NAME NeoHotKeyPlg
#include <pluginexport.cpp>

NeoHotKeyPlg::NeoHotKeyPlg(YJson& settings):
  PluginObject(InitSettings(settings), u8"neohotkeyplg", u8"热键注册")
{
  InitFunctionMap();
}

NeoHotKeyPlg::~NeoHotKeyPlg()
{
  //
}

void NeoHotKeyPlg::InitFunctionMap()
{
  //
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
    {u8"Hotkeys", YJson::O {
      {u8"开关翻译窗口", YJson::O {
        {u8"Keys", YJson::A { "Shift", "Z" }},
        {u8"Enabled", false},
      }},
      {u8"屏幕截图", YJson::O {
        {u8"Keys", YJson::A { "Ctrl", "Shift", "A" }},
        {u8"Enabled", false},
      }},
    }},
  };
}
