#include <weatheritem.hpp>
#include <weatherdlg.h>

#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QLabel>

std::map<std::u8string, int> WeatherItem::m_FontsMap;

WeatherItem::WeatherItem(QWidget* parent)
  : QWidget(parent)
{
  LoadFontsMap();
  SetupUi();
  setFixedSize(200, 100);
}

WeatherItem::~WeatherItem()
{
  //
}

void WeatherItem::LoadFontsMap()
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

void WeatherItem::SetupUi()
{
  auto mainLayout = QVBoxLayout(this);
  auto label = new QLabel(this);
  label->setObjectName(WeatherDlg::m_FontObjectName);
  label->setFont(WeatherDlg::m_WeatherFontName);
  label->setText(QChar(m_FontsMap[u8"thunder-storm"]));
  mainLayout.addWidget(label);
}
