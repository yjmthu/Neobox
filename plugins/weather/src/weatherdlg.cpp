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
#include <weatherh.h>
#include <weatherd.hpp>
#include <savedcity.hpp>

#include <QLabel>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QFile>
#include <QScrollArea>
#include <QTimer>
#include <QFontDatabase>
#include <QCursor>
#include <QPushButton>
#include <QEventLoop>

#include <chrono>

QString WeatherDlg::m_WeatherFontName;
std::map<std::u8string, int> WeatherDlg::m_FontsMap;

void WeatherDlg::showEvent(QShowEvent *event)
{
  using namespace std::chrono;
  static auto timePrev = time_point<system_clock, seconds>(0s);
  auto timeNow = time_point_cast<seconds>(system_clock::now());
  auto timeDelta = timeNow - timePrev;
  timePrev = timeNow;
  if (timeDelta > 300s) {
    QTimer::singleShot(500, [this](){ m_Weather->Fetch(Weather::GetTypes::Now); });
  }

  m_SearchList->Move();
  // m_Hours->m_ScrollArea->setMinimumWidth(m_Item->width());
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
  , m_Hours(new WeatherH(this))
  , m_Days(new WeatherD(this))
  , m_CityName(new QPushButton(GetCityName(), m_CenterWidget))
  , m_CityEdit(new CitySearch(m_CenterWidget))
  , m_SearchList(new CityList(m_CenterWidget, m_Config, *m_CityName, *m_CityEdit))
  , m_Update(new QPushButton(this))
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

void WeatherDlg::LoadFontsMap()
{
  QFile file(":/fonts/qweather-icons.json");

  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  auto data = file.readAll();
  YJson json(data.begin(), data.end());

  for (auto& [name, value]: json.getObject()) {
    m_FontsMap[name] = value.getValueInt();
  }
}

void WeatherDlg::LoadStyleSheet()
{
  QFile file(":/styles/WeatherStyle.qss");
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
  m_CenterWidget->setObjectName("Weather");
  m_CityName->setObjectName("CityName");
  m_CityName->setCursor(Qt::PointingHandCursor);

  AddTitle("Neobox 天气预报");
  AddCloseButton();
  SetShadowAround(m_CenterWidget);

  // setFixedSize(520, 520);
  setFixedWidth(520);
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

  auto l = new QHBoxLayout();
  l->setContentsMargins(0, 0, 30, 0);
  layout->addLayout(l);
  QPixmap pix(":/icons/Location.png");
  auto location = new QLabel(this);
  location->setFixedSize(16, 16);
  location->setPixmap(pix.scaled(location->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
  l->addWidget(location);
  l->addWidget(m_CityName);
  m_Update->setCursor(Qt::PointingHandCursor);
  m_Update->setObjectName("WeatherUpdate");
  m_Update->setFixedSize(18, 18);
  l->addWidget(m_Update);

  l->addStretch();
  AddScrollBar(m_SearchList->verticalScrollBar(), false);
  AddScrollBar(m_SearchList->horizontalScrollBar(), true);
  l->addWidget(m_CityEdit);

  layout->addWidget(m_Item);
  layout->addWidget(new QLabel("<h4>24小时预报</h4>", this));
  layout->addWidget(m_Hours);
  AddScrollBar(m_Hours->horizontalScrollBar(), true);
  AddScrollBar(m_Hours->verticalScrollBar(), true);
  layout->addWidget(new QLabel("<h4>未来7日预报</h4>", this));
  layout->addWidget(m_Days);
  AddScrollBar(m_Days->verticalScrollBar(), false);

  m_SearchList->raise();
  m_SearchList->hide();
}

void WeatherDlg::LoadQweatherFonts()
{
  int fontId = QFontDatabase::addApplicationFont(":/fonts/qweather-icons.ttf");
  QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
  m_WeatherFontName = fontFamilies.at(0);

  LoadFontsMap();
}

void WeatherDlg::ConnectAll() {
  connect(m_Weather, &Weather::Finished, this, [this](int type, bool succeed) {
    using enum Weather::GetTypes;
    switch (static_cast<Weather::GetTypes>(type)) {
    case Cities:
      ShowCityList(succeed);
      break;
    case Hours:
      UpdateHours(succeed);
      if (succeed) m_Weather->Fetch(Days);
      break;
    case Days:
      UpdateDays(succeed);
      if (succeed) mgr->ShowMsg("更新成功！");
      break;
    case Now:
      UpdateItem(succeed);
      if (succeed) m_Weather->Fetch(Hours);
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
  connect(m_CityName, &QPushButton::clicked, this, [this](){
    static SavedCity* editor = nullptr;
    if (editor) {
      editor->show();
      editor->raise();
      return ;
    }
    editor = new SavedCity(this, m_Config);
    editor->move(this->pos() + QPoint(50, 50));
    auto loop = QEventLoop(this);
    connect(editor, &SavedCity::Finished, this, [this, &loop](bool quit) {
      if (quit) {
        loop.quit();
      } else {
        m_CityName->setText(GetCityName());
        m_Weather->Fetch(Weather::GetTypes::Now);
      }
    });
    connect(m_SearchList, &CityList::CityChanged, editor, &SavedCity::Update);
    editor->show();
    loop.exec();
    editor = nullptr;
  });

  connect(m_Update, &QPushButton::clicked, this, [this]() {
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
  } else {
    m_SearchList->hide();
  }
}

void WeatherDlg::UpdateItem(bool succeed)
{
  auto result = m_Weather->Get();
  if (succeed && result) {
    m_Item->SetJSON(*result);
  }
}

void WeatherDlg::UpdateHours(bool succeed)
{
  auto result = m_Weather->Get();
  if (succeed && result) {
    m_Hours->SetJSON(*result);
  }
}

void WeatherDlg::UpdateDays(bool succeed)
{
  auto result = m_Weather->Get();

  if (succeed && result) {
    m_Days->SetJSON(*result);
  }
}