#include <weatherdlg.h>
#include <pluginmgr.h>
#include <neomenu.hpp>
#include <yjson.h>

#include <QHBoxLayout>

WeatherDlg::WeatherDlg(YJson& settings)
  : WidgetBase(mgr->m_Menu, false, false)
  , m_CenterWidget(new QWidget(this))
  , m_Settings(settings)
{
  SetupUi();
}

WeatherDlg::~WeatherDlg() {
  //
}

void WeatherDlg::SetupUi()
{
  setWindowTitle("天气预报");
  setAttribute(Qt::WA_TranslucentBackground);

  auto const mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(m_CenterWidget);

  AddTitle("天气预报");
  AddCloseButton();
  AddMinButton();
  SetShadowAround(m_CenterWidget);
}
