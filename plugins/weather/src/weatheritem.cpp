#include <weatheritem.hpp>
#include <weatherdlg.h>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFile>
#include <QFont>
#include <QLabel>

using namespace std::literals;

WeatherItem::WeatherItem(QWidget* parent)
  : QFrame(parent)
  , m_IconDayNight(new QLabel(this))
  , m_TextDayNight(new QLabel(this))
  , m_Temperature(new QLabel(this))
{
  SetObjectName();
  SetupUi();
}

WeatherItem::~WeatherItem()
{
  //
}

void WeatherItem::SetupUi()
{
  constexpr auto iIconSize = 64;
  setFixedHeight(100);
  auto mainLayout = new QHBoxLayout(this);
  mainLayout->setSpacing(10);
  auto layout = new QVBoxLayout;
  layout->setContentsMargins(11, 8, 11, 8);
  layout->setSpacing(3);

  m_IconDayNight->setAlignment(Qt::AlignCenter);
  m_IconDayNight->setFixedSize(iIconSize, iIconSize);
  m_IconDayNight->setText(u8"🌴");
  mainLayout->addWidget(m_IconDayNight);
  
  m_Temperature->setText("26°");
  layout->addWidget(m_Temperature);

  m_TextDayNight->setText(u8"天晴有雨");
  m_TextDayNight->setFixedWidth(iIconSize);
  m_TextDayNight->setAlignment(Qt::AlignCenter);
  layout->addWidget(m_TextDayNight);

  mainLayout->addLayout(layout);

  mainLayout->addLayout(SetupGrid());
  mainLayout->addStretch();
}

class QGridLayout* WeatherItem::SetupGrid()
{
  typedef WeatherDetail W;
  m_DetailMap = {
    {u8"windScale", W { u8"dust-raising-winds"s, "风力等级", u8"2", "级" }},
    {u8"windDir", W { u8"fresh-wind"s, "风向", u8"南风", "" }},
    {u8"feelsLike", W { u8"high-temperature"s, "体感温度", u8"30", "℃" }},
    {u8"humidity", W { u8"low-humidity"s, "相对湿度", u8"74", "%" }},
    {u8"vis", W { u8"low-visibility"s, "能见度", u8"22", "千米" }},
    {u8"pressure", W { u8"fog"s, "大气压强", u8"1003", "百帕" }},
    // {u8"precip", W { u8"flooding"s, u8"时降水"s, u8"0.0", u8"毫米" }},
  };
  constexpr auto iRowSize = 2;
  auto curIndex = 0;
  auto grid = new QGridLayout;

  for (auto& [key, item]: m_DetailMap) {
    auto curRow = curIndex / iRowSize;
    auto offset = (curIndex++ % iRowSize) * 3;
    auto icon = new QLabel(QChar(WeatherDlg::m_FontsMap[item.icon]), this);
    icon->setObjectName("IconExponent");
    grid->addWidget(icon, curRow, offset);
    auto lName = new QLabel(item.name);
    lName->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    grid->addWidget(lName, curRow, ++offset);
    item.label->setParent(this);
    item.label->setAlignment(Qt::AlignCenter);
    grid->addWidget(item.label, curRow, ++offset);
    item.SetLabelText();
  }

  return grid;
}

void WeatherItem::SetObjectName()
{
  setObjectName("WeatherItem");
  m_IconDayNight->setObjectName("IconDayNight");
  m_TextDayNight->setObjectName("TextDayNight");
  m_Temperature->setObjectName("Temperature");
}

void WeatherItem::SetJSON(const YJson& json)
{
  auto now = json[u8"now"];

  for (auto& [key, value]: now.getObject()) {
    auto iter = m_DetailMap.find(key);
    if (iter == m_DetailMap.end()) continue;
    auto& item = iter->second;
    item.count = value.getValueString();
    item.SetLabelText();
  }

  auto& temp = now[u8"temp"].getValueString();
  auto& text = now[u8"text"].getValueString();
  auto& icon = now[u8"icon"].getValueString();

  SetTemp(temp);
  SetDayText(text);
  SetDayIcon(icon);
}


void WeatherItem::SetTemp(std::u8string_view value)
{
  QString temp = QString::fromUtf8(value.data(), value.size());
  m_Temperature->setText(QStringLiteral("%1°").arg(temp));
}

void WeatherItem::SetDayText(std::u8string_view value)
{
  auto text = QString::fromUtf8(value.data(), value.size());
  m_TextDayNight->setText(text);
}

void WeatherItem::SetDayIcon(const std::u8string& value)
{
  auto unicode = WeatherDlg::m_FontsMap[value];
  m_IconDayNight->setText(QChar(unicode));
}