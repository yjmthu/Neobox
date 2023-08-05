#include <weatherdlg.h>
#include <pluginmgr.h>
#include <neomenu.hpp>
#include <yjson.h>
#include <weather.hpp>
#include <pluginmgr.h>

#include <QPlainTextEdit>
#include <QHBoxLayout>

WeatherDlg::WeatherDlg(YJson& settings)
  : WidgetBase(mgr->m_Menu, false, false)
  , m_CenterWidget(new QWidget(this))
  , m_Settings(settings)
  , m_Weather(new Weather)
{
  SetupUi();
  InitComponent();
  ConnectAll();
  m_Weather->SendRequest();
}

WeatherDlg::~WeatherDlg() {
  delete m_Weather;
}

void WeatherDlg::SetupUi()
{
  setWindowTitle("天气预报");
  setAttribute(Qt::WA_TranslucentBackground);
  setWindowFlag(Qt::Tool);

  auto const mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(m_CenterWidget);
  m_CenterWidget->setStyleSheet(
    "background-color: white;"
    "border-radius: 6px;"
  );

  AddTitle("天气预报");
  AddCloseButton();
  AddMinButton();
  SetShadowAround(m_CenterWidget);

  setFixedSize(500, 360);
}

void WeatherDlg::InitComponent()
{
  auto layout = new QVBoxLayout(m_CenterWidget);
  layout->setContentsMargins(11, 30, 11, 11);
  m_Text = new QPlainTextEdit(m_CenterWidget);
  layout->addWidget(m_Text);
  AddScrollBar(m_Text->verticalScrollBar());
}

void WeatherDlg::ConnectAll() {
  connect(m_Weather, &Weather::Finished, this, [this](bool succeed) {
    auto result = m_Weather->GetResult();
    if (succeed && result) {
      auto str = result->toString(true);
      m_Text->setPlainText(QString::fromUtf8(str.data(), str.size()));
    } else {
      mgr->ShowMsg("获取天气数据失败！");
    }
  });
}
