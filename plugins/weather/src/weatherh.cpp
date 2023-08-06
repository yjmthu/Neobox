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
  : QWidget(parent)
  , m_ScrollArea(new QScrollArea(this))
  , m_ScrollWidget(new QWidget(this))
{
  SetupUi();
}

WeatherH::~WeatherH()
{
  //
}

void WeatherH::SetupUi()
{
  auto mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(new QLabel("<h4>24小时预报</h4>", this));
  mainLayout->addWidget(m_ScrollArea);
  mainLayout->setContentsMargins(0,0,0,0);
  // m_ScrollArea->setBackgroundRole(QPalette::ColorRole::Dark);
  m_ScrollArea->setFixedHeight(190);
  m_ScrollWidget->setFixedHeight(180);
  m_ScrollWidget->setObjectName("ScrollWidget");
  auto layout = new QHBoxLayout(m_ScrollWidget);
  layout->setContentsMargins(10, 10, 10, 10);
  m_Data.reserve(24);
  for (int i=0; i!=24; ++i) {
    auto w = new QWidget(m_ScrollWidget);
    w->setFixedSize(40, 160);
    w->setStyleSheet("background-color:transparent;");
    auto l = new QVBoxLayout(w);
    l->setContentsMargins(0,0,0,0);
    l->setAlignment(Qt::AlignCenter);
    m_Data.emplace_back(this, l, i);
    layout->addWidget(w);
  }
  m_ScrollArea->setWidget(m_ScrollWidget);
  // m_ScrollArea->setAlignment(Qt::AlignCenter);
  m_ScrollWidget->installEventFilter(this);
}

void WeatherH::SetJSON(const YJson& json)
{
  auto hourly = json[u8"hourly"];

  auto iter = m_Data.begin();
  for (auto& item: hourly.getArray()) {
    iter->ParseJSON(item);
    ++iter;
  }
}

bool WeatherH::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::Wheel && watched == m_ScrollWidget) {
    auto ev = reinterpret_cast<QWheelEvent*>(event);
    auto bar = m_ScrollArea->horizontalScrollBar();
    auto delta = ev->angleDelta().y() / 120;
    bar->setValue(bar->value() - delta * 70);
    return true;
  }

  return QWidget::eventFilter(watched, event);
}
