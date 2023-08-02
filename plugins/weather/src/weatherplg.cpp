#include <weatherplg.h>
#include <weatherdlg.h>

#define PluginName WeatherPlg
#include <pluginexport.cpp>

PluginName::PluginName(YJson& settings)
  : PluginObject(InitSettings(settings), u8"weatherplg", u8"天气预报")
  , m_WeatherDlg(new WeatherDlg(m_Settings))
{
  InitFunctionMap();
}

PluginName::~PluginName() {
  delete m_WeatherDlg;
}

void PluginName::InitFunctionMap()
{
  m_PluginMethod = {
    {u8"openWindow",
      {u8"开关窗口", u8"打开/关闭 天气预报界面。", [this](PluginEvent, void*) {
        if (m_WeatherDlg->isVisible()) {
          m_WeatherDlg->hide();
        } else {
          m_WeatherDlg->show();
        }
      }, PluginEvent::Void}
    },
  };
}

QAction* PluginName::InitMenuAction()
{
  return PluginObject::InitMenuAction();
}

YJson& PluginName::InitSettings(YJson& settings)
{
  if (!settings.isObject()) {
    settings = YJson::O {
      {u8"ApiData", YJson::Object},
    };
  }
  return settings;
}
