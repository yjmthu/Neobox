#include <weatherd.hpp>

WeatherD::WeatherD(QWidget* parent)
  : QScrollArea(parent)
{
  SetupUi();
}

WeatherD::~WeatherD()
{
  //
}

void WeatherD::SetJSON(YJson& json)
{
  auto daily = json[u8"daily"];

  auto iter = m_Data.begin();
  int min = 100, max = -100;
  for (auto& item: daily.getArray()) {
    auto& ji = item[u8"tempMin"];
    auto& ja = item[u8"tempMax"];
    auto ii = std::stoi(std::string(ji.getValueString().begin(), ji.getValueString().end()));
    auto ia = std::stoi(std::string(ja.getValueString().begin(), ja.getValueString().end()));
    ji = ii;
    ja = ia;
    min = std::min(min, ii);
    max = std::max(max, ia);
  }
  for (auto& item: daily.getArray()) {
    iter->ParseJSON(item, min, max);
    ++iter;
  }
}

void WeatherD::SetupUi()
{
  setFixedHeight(200);
  auto widget = new QWidget(this);
  widget->setObjectName("ScrollWidget");
  auto layout = new QVBoxLayout(widget);
  layout->setContentsMargins(10, 10, 10, 10);
  m_Data.reserve(7);
  for (int i=0; i!=7; ++i) {
    auto w = new QWidget(widget);
    w->setStyleSheet("background-color: transparent;");
    m_Data.emplace_back(w, i);
    layout->addWidget(w);
  }
  setWidget(widget);
  // m_ScrollArea->setAlignment(Qt::AlignCenter);
  widget->installEventFilter(this);
}
