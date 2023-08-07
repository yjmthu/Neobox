#include <weatherh.hpp>
#include <weatherdlg.h>
#include <pluginmgr.h>

#include <QLabel>
#include <QEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QWheelEvent>
#include <QScrollBar >

WeatherH::WeatherH(QWidget* parent)
  : QScrollArea(parent)
{
  SetupUi();
}

WeatherH::~WeatherH()
{
  //
}

void WeatherH::SetupUi()
{
  setFixedHeight(190);
  // m_ScrollArea->setBackgroundRole(QPalette::ColorRole::Dark);
  auto widget = new QWidget(this);
  widget->setFixedHeight(180);
  widget->setObjectName("ScrollWidget");
  auto layout = new QHBoxLayout(widget);
  layout->setContentsMargins(10, 10, 10, 10);
  m_Data.reserve(24);
  for (int i=0; i!=24; ++i) {
    auto w = new QWidget(widget);
    w->setFixedSize(40, 160);
    w->setStyleSheet("background-color:transparent;");
    auto l = new QVBoxLayout(w);
    l->setContentsMargins(0,0,0,0);
    l->setAlignment(Qt::AlignCenter);
    m_Data.emplace_back(this, l, i);
    layout->addWidget(w);
  }
  setWidget(widget);
  // m_ScrollArea->setAlignment(Qt::AlignCenter);
  widget->installEventFilter(this);
}

void WeatherH::SetJSON(const YJson& json)
{
  auto hourly = json[u8"hourly"];

  int min = 100, max = -100;

  for (auto& item: hourly.getArray()) {
    auto& temp = item[u8"temp"];
    auto iTemp = std::stoi(std::string(temp.getValueString().begin(), temp.getValueString().end()));
    min = std::min(min, iTemp);
    max = std::max(max, iTemp);
    temp = iTemp;
  }

  min -= 3;

  auto iter = m_Data.begin();
  for (auto& item: hourly.getArray()) {
    iter->ParseJSON(item, min, max);
    ++iter;
  }
}

bool WeatherH::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::Wheel && watched == this->widget()) {
    auto ev = reinterpret_cast<QWheelEvent*>(event);
    auto const bar = horizontalScrollBar();
    auto delta = ev->angleDelta().y() / 120;
    bar->setValue(bar->value() - delta * 70);
    return true;
  }

  return QWidget::eventFilter(watched, event);
}
