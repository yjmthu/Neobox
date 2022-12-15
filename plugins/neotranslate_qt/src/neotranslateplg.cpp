#include <neotranslateplg.h>
#include <translatedlg.h>
#include <yjson.h>

#include <QTextEdit>

#define CLASS_NAME NeoTranslatePlg
#include <pluginexport.cpp>

/*
 * NeoTranslatePlugin
 */

NeoTranslatePlg::NeoTranslatePlg(YJson& settings):
  PluginObject(InitSettings(settings), u8"neotranslateplg", u8"极简翻译"),
  m_TranslateDlg(new NeoTranslateDlg(settings))
{
  AddMainObject(m_TranslateDlg);
  InitFunctionMap();
  m_Following.push_back({u8"neospeedboxplg", [this](PluginEvent event, void* data){
    if (event == PluginEvent::MouseMove) {
      if (m_TranslateDlg->isVisible())
        m_TranslateDlg->hide();
    } else if (event == PluginEvent::MouseDoubleClick) {
      if (m_TranslateDlg->isVisible())
        m_TranslateDlg->hide();
      else
        m_TranslateDlg->show();
    }
  }});
  m_Following.push_back({u8"neoocrplg", [this](PluginEvent event, void* data){
    if (event == PluginEvent::U8string) {
      if (!data) return;
      const auto& str = *reinterpret_cast<std::u8string*>(data);
      auto const txtfrom = m_TranslateDlg->findChild<QTextEdit*>("neoTextFrom");
      txtfrom->setPlainText(Utf82QString(str));
      m_TranslateDlg->show();
    }
  }});
}

NeoTranslatePlg::~NeoTranslatePlg()
{
  delete m_TranslateDlg;  // must delete the dlg when plugin destroying.
}

void NeoTranslatePlg::InitFunctionMap() {
  m_PluginMethod = {
    {u8"toggleVisibility",
      {u8"打开窗口", u8"打开或关闭极简翻译界面", [this](PluginEvent, void* data){
        m_TranslateDlg->ToggleVisibility();
      }, PluginEvent::Void
    }},
    {u8"enableReadClipboard",
      {u8"读剪切板", u8"打开界面自动读取剪切板内容到From区", [this](PluginEvent event, void* data) {
        if (event == PluginEvent::Bool) {
          m_Settings[u8"AutoTranslate"] = *reinterpret_cast<bool*>(data);
          mgr->SaveSettings();
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool*>(data) = m_Settings[u8"AutoTranslate"].isTrue();
        }
      }, PluginEvent::Bool}
    },
    {u8"enableAutoTranslate",
      {u8"自动翻译", u8"打开界面自动翻译From区内容", [this](PluginEvent event, void* data) {
        if (event == PluginEvent::Bool) {
          m_Settings[u8"ReadClipboard"] =  *reinterpret_cast<bool*>(data);
          mgr->SaveSettings();
        } else if (event == PluginEvent::Bool) {
          *reinterpret_cast<bool*>(data) = m_Settings[u8"ReadClipboard"].isTrue();
        }
      }, PluginEvent::Bool}
    },
  };
}

void NeoTranslatePlg::InitMenuAction()
{
  PluginObject::InitMenuAction();
}

YJson& NeoTranslatePlg::InitSettings(YJson& settings)
{
  if (settings.isObject()) return settings;
  return settings = YJson::O {
    { u8"Mode", 0 },
    { u8"AutoTranslate", false },
    { u8"ReadClipboard", false },
  };
  // we may not need to call SaveSettings;
}
