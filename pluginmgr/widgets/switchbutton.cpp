#include "switchbutton.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QBrush>
#include <QAbstractAnimation>
#include <QVariantAnimation>

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
  , m_Animation(new QVariantAnimation(this))
{
  setFixedSize(QSize(::w, ::h));
  setCursor(Qt::PointingHandCursor);

  m_Animation->setStartValue(from);
  m_Animation->setEndValue(to);
  m_Animation->setDuration(150);

  connect(m_Animation, &QVariantAnimation::valueChanged, this, [this](const QVariant & val){
    value = val.toInt();
    update();
  });
}

SwitchButton::~SwitchButton()
{
  delete m_Animation;
}

void SwitchButton::SetChecked(bool on, bool animate)
{
  if (on == checked) {
    return;
  }
  checked = on;
  auto const direction = checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;
  auto const running = m_Animation->state() == QAbstractAnimation::Running;
  if (animate) {
    m_Animation->setDirection(direction);
    if(running) {
      m_Animation->pause();
      m_Animation->resume();
    } else {
      m_Animation->start(QAbstractAnimation::KeepWhenStopped);
    }
    update();
  } else {
    if (running) {
      m_Animation->pause();
    }
    value = checked ? ::to : ::from;
    update();
  }
}

bool SwitchButton::IsChecked() const
{
  return checked;
}

void SwitchButton::Toggle(bool animate)
{
  SetChecked(!checked, animate);
}

void SwitchButton::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::transparent);
  if (value == ::to) {
    painter.setBrush(QBrush(QColor("#a8c3d5")));
    painter.drawRoundedRect(rect(), h / 2.0, h / 2.0);
    painter.setBrush(QBrush(QColor(10, 180, 10)));
  } else {
    painter.setBrush(QBrush(QColor(72, 72, 72)));
    painter.drawRoundedRect(rect(), h / 2.0, h / 2.0);
    painter.setBrush(QBrush(QColor(209, 209, 209)));
  }
  painter.drawEllipse(value, d, r * 2, r * 2);
}

void SwitchButton::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    Toggle(true);
    emit Clicked(checked);
  }

  return QWidget::mousePressEvent(event);
}
