#include "switchbutton.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QBrush>
#include <QTimer>

static constexpr int h = 20;
static constexpr int w = h * 2;
static constexpr int r = 9;

SwitchButton::SwitchButton(QWidget* parent)
  : QWidget(parent)
  , checked(false)
  , value(0)
{
  setFixedSize(QSize(::w, ::h));
}

SwitchButton::~SwitchButton()
{
}

void SwitchButton::SetChecked(bool on)
{
  int to, d;
  if ((this->checked = on)) {
    if (value != 0) {
      return;
    }
    to = r;
    d = 1;
  } else {
    if (value != r) {
      return;
    }
    to = 0;
    d = -1;
  }

  auto const timer = new QTimer;
  connect(timer, &QTimer::timeout, this, [this, to, d, timer](){
    value += d;
    if (value == to) {
      timer->stop();
      timer->deleteLater();
    }
    update();
  });
  timer->start(15);
}

bool SwitchButton::Checked() const
{
  return checked;
}

void SwitchButton::paintEvent(QPaintEvent* event)
{
  constexpr int delta = h / 2 - r;
  QPainter painter(this);

  if (value) {
    painter.setBrush(QBrush(Qt::red));
  } else {
    painter.setBrush(QBrush(Qt::gray));
  }
  painter.drawRoundedRect(rect(), ::r, ::r);
  painter.translate(delta, delta);
  painter.setBrush(QBrush(Qt::blue));
  painter.drawEllipse(value, 0, r * 2, r * 2);
}

