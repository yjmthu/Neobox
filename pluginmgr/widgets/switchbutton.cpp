#include "switchbutton.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QBrush>
#include <QTimer>

static constexpr int r = 9;
static constexpr int d = 1;
static constexpr int h = 2 * (r + d);
static constexpr int w = 3 * (r + d) + r;
static constexpr int from = d;
static constexpr int to = w - d - 2 * r;

SwitchButton::SwitchButton(QWidget* parent)
  : QWidget(parent)
  , checked(false)
  , value(::from)
{
  setFixedSize(QSize(::w, ::h));
  setCursor(Qt::PointingHandCursor);
}

SwitchButton::~SwitchButton()
{
}

void SwitchButton::SetChecked(bool on)
{
  int x, k;
  if ((this->checked = on)) {
    if (value != ::from) {
      return;
    }
    x = ::to;
    k = 1;
  } else {
    if (value != ::to) {
      return;
    }
    x = ::from;
    k = -1;
  }

  auto const timer = new QTimer;
  connect(timer, &QTimer::timeout, this, [this, x, k, timer](){
    value += k;
    if (value == x) {
      timer->stop();
      timer->deleteLater();
    }
    update();
  });
  timer->start(5);
}

bool SwitchButton::IsChecked() const
{
  return checked;
}

void SwitchButton::Toggle()
{
  SetChecked(!checked);
}

void SwitchButton::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::transparent);
  if (value == ::to) {
    painter.setBrush(QBrush(QColor(202, 147, 211)));
    painter.drawRoundedRect(rect(), h / 2.0, h / 2.0);
    painter.setBrush(QBrush(QColor(209, 209, 209)));
  } else {
    painter.setBrush(QBrush(QColor(72, 72, 72)));
    painter.drawRoundedRect(rect(), h / 2.0, h / 2.0);
    painter.setBrush(QBrush(QColor(0, 0, 0)));
  }
  painter.drawEllipse(value, d, r * 2, r * 2);
}

void SwitchButton::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    Toggle();
    emit Clicked(checked);
  }

  return QWidget::mousePressEvent(event);
}
