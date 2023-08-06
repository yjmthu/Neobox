#include <weatherdlg.h>
#include <pluginmgr.h>
#include <neomenu.hpp>
#include <yjson.h>
#include <weather.hpp>
#include <pluginmgr.h>
#include <weathercfg.h>
#include <citysearch.hpp>
#include <citylist.hpp>
#include <weatheritem.hpp>

#include <QLabel>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QFile>

QString WeatherDlg::m_WeatherFontName;

void WeatherDlg::showEvent(QShowEvent *event)
{
  m_SearchList->Move();
  WidgetBase::showEvent(event);
}

void WeatherDlg::hideEvent(QHideEvent *event)
{
  if (pos() != m_LastPosition) {
    m_LastPosition = pos();
    m_Config.SetWindowPosition({
      m_LastPosition.x(), m_LastPosition.y()
    });
  }
  WidgetBase::hideEvent(event);
}

WeatherDlg::WeatherDlg(WeatherCfg& settings)
  : WidgetBase(mgr->m_Menu, false, false)
  , m_CenterWidget((LoadQweatherFonts(), new QWidget(this)))
  , m_Config(settings)
  , m_Weather(new Weather(m_Config))
  , m_Item(new WeatherItem(this))
  , m_CityName(new QLabel(GetCityName(), m_CenterWidget))
  , m_CityEdit(new CitySearch(m_CenterWidget))
  , m_SearchList(new CityList(m_CenterWidget, m_Config, *m_CityName, *m_CityEdit))
{
  SetupUi();
  LoadStyleSheet();
  InitComponent();
  ConnectAll();
}

WeatherDlg::~WeatherDlg() {
  delete m_SearchList;
  delete m_CityEdit;
  delete m_CityName;
  delete m_Weather;
}

void WeatherDlg::LoadStyleSheet()
{
  QFile file(":/styles/WeatherStyle.css");
  if (!file.open(QIODevice::ReadOnly)) {
    mgr->ShowMsgbox(L"NO", L"打开文件失败");
    return;
  }
  QString sheet = file.readAll();
  setStyleSheet(sheet.arg(m_WeatherFontName));
  file.close();
  return;
}

void WeatherDlg::SetupUi()
{
  setWindowTitle("天气预报");
  setAttribute(Qt::WA_TranslucentBackground);
  setWindowFlag(Qt::Tool);

  auto const mainLayout = new QHBoxLayout(this);
  mainLayout->addWidget(m_CenterWidget);
  m_CenterWidget->setObjectName("whiteBackground");

  AddTitle("天气预报");
  AddCloseButton();
  AddMinButton();
  SetShadowAround(m_CenterWidget);

  setFixedSize(520, 500);
  auto pos = m_Config.GetWindowPosition();
  if (pos.front().isNumber() && pos.back().isNumber()) {
    m_LastPosition = {pos.front().getValueInt(), pos.back().getValueInt()};
    move(m_LastPosition);
  }
}

void WeatherDlg::InitComponent()
{
  auto layout = new QVBoxLayout(m_CenterWidget);
  layout->setContentsMargins(11, 30, 11, 11);
  m_Text = new QPlainTextEdit(m_CenterWidget);

  auto l = new QHBoxLayout();
  l->setContentsMargins(0, 0, 30, 0);
  layout->addLayout(l);
  l->addWidget(m_CityName);

  l->addStretch();
  AddScrollBar(m_SearchList->verticalScrollBar(), false);
  AddScrollBar(m_SearchList->horizontalScrollBar(), true);
  l->addWidget(m_CityEdit);

  layout->addWidget(m_Item);
  layout->addWidget(m_Text);
  AddScrollBar(m_Text->verticalScrollBar());

  m_SearchList->raise();
  m_SearchList->hide();
}

void WeatherDlg::LoadQweatherFonts()
{
  int fontId = QFontDatabase::addApplicationFont(":/fonts/qweather-icons.ttf");
  QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
  m_WeatherFontName = fontFamilies.at(0);
}

void WeatherDlg::ConnectAll() {
  connect(m_Weather, &Weather::Finished, this, [this](Weather::GetTypes type, bool succeed) {
    using enum Weather::GetTypes;
    switch (type) {
    case Cities:
      ShowCityList(succeed);
      break;
    case Hours:
      break;
    case Days:
      break;
    case Now:
      UpdateItem(succeed);
      break;
    }
  });

  connect(m_CityEdit, &QLineEdit::textChanged, this, &WeatherDlg::SearchCities);
  connect(m_CityEdit, &QLineEdit::returnPressed, this, [this] (){
    SearchCities(m_CityEdit->text());
  });
  connect(m_CityEdit, &CitySearch::FocusIn, m_SearchList, &CityList::Show);
  connect(m_CityEdit, &CitySearch::FocusOut, m_SearchList, &QListWidget::hide);
  connect(m_SearchList, &CityList::CityChanged, [this]() {
    m_Weather->Fetch(Weather::GetTypes::Now);
  });
}

QString WeatherDlg::GetCityName() const
{
  auto object = m_Config.GetCityList();
  auto id = m_Config.GetCity();
  auto text = CityInfo::FromJSON(id, object[id]).GetCityName();

  return QString::fromUtf8(text.data(), text.size());
}

void WeatherDlg::SearchCities(QString city)
{
  city = city.trimmed();
  if (city.isEmpty()) {
    m_Text->clear();
    return;
  }
  auto data = city.toUtf8();
  std::u8string_view view(reinterpret_cast<const char8_t*>(data.data()), data.size());
  m_Weather->Fetch(Weather::GetTypes::Cities, view);
}

void WeatherDlg::ShowCityList(bool succeed)
{
  auto result = m_Weather->Get();
  if (succeed && result) {
    m_SearchList->SetConetnt(*result);
    m_SearchList->show();
    auto str = result->toString(true);
    m_Text->setPlainText(QString::fromUtf8(str.data(), str.size()));
  } else {
    m_SearchList->hide();
  }
}

void WeatherDlg::UpdateItem(bool succeed)
{
  auto result = m_Weather->Get();
  if (succeed && result) {
    m_Item->SetJSON(*result);
    auto str = result->toString(true);
    m_Text->setPlainText(QString::fromUtf8(str.data(), str.size()));
  }
}
