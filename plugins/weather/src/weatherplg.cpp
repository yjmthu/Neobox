#include <weatherplg.h>
#include <weatherdlg.h>
#include <neomenu.hpp>

#include <QMessageBox>

#define PluginName WeatherPlg
#include <pluginexport.cpp>

PluginName::PluginName(YJson& settings)
  : PluginObject(InitSettings(settings), u8"weatherplg", u8"天气预报")
  , m_Config(settings)
  , m_WeatherDlg(new WeatherDlg(m_Config))
{
  InitFunctionMap();
}

PluginName::~PluginName() {
  delete m_MainMenuAction;
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
    {u8"setApiKey",
      {u8"设置密钥", u8"填写和风天气API密钥。", [this](PluginEvent, void*) {
        const auto key = m_Config.GetApiKey();
        auto result = mgr->m_Menu->GetNewU8String("输入", "输入和风天气密钥", key);
        if (!result) return;
        
        if (*result != key) {
          m_Config.SetApiKey(*result, false);
        }
        auto res = QMessageBox::question(m_WeatherDlg, u8"提示", u8"您是否为付费用户？");
        m_Config.SetIsPaidUser(res == QMessageBox::Yes, true);
        mgr->ShowMsg("保存成功！");
      }, PluginEvent::Void}
    },
    {u8"clearApiKey",
      {u8"清除密钥", u8"清除您的密钥。", [this](PluginEvent, void*) {
        m_Config.SetApiKey(u8"", false);
        m_Config.SetIsPaidUser(false, true);
        mgr->ShowMsg(u8"清除成功！");
      }, PluginEvent::Void}
    },
  };
}

QAction* PluginName::InitMenuAction()
{
  m_MainMenuAction = new QAction("天气预报");
  m_MainMenuAction->setToolTip("打开天气预报窗口");
  QObject::connect(m_MainMenuAction, &QAction::triggered, m_WeatherDlg, &WeatherDlg::show);
  PluginObject::InitMenuAction();

  return m_MainMenuAction;
}

YJson& PluginName::InitSettings(YJson& settings)
{
  if (!settings.isObject()) {
    settings = YJson::O {
      {u8"ApiKey", YJson::String},
      {u8"CityList", YJson::O {
        {u8"101010100", { u8"北京市", u8"北京", u8"北京" }}
      }},
      {u8"City", u8"101010100"},
      {u8"Prompt", false},
      {u8"UpdateCycle", 60},
      {u8"IsPaidUser", false},
      {u8"WindowPosition", YJson::A { nullptr, nullptr }},
    };
  }
  return settings;
}
