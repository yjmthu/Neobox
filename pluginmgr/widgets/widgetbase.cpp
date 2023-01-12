#include <widgetbase.hpp>

#include <QPushButton>
#include <QMouseEvent>

WidgetBase::WidgetBase(QWidget* parent)
  : QWidget(parent, Qt::FramelessWindowHint)
{
  setWindowIcon(QIcon(QStringLiteral(":/icons/neobox.ico")));
}

WidgetBase::~WidgetBase()
{
  //
}


void WidgetBase::showEvent(QShowEvent *event)
{
  QPoint point(width() - 25, 12);
  for (auto button: m_Buttons) {
    button->move(point);
    point.rx() -= 30;
  }
}

void WidgetBase::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    m_ConstPos = event->pos();
    setMouseTracking(true);
  }
}

void WidgetBase::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    setMouseTracking(false);
  }
}

void WidgetBase::mouseMoveEvent(QMouseEvent* event)
{
  if (event->buttons() == Qt::LeftButton) {
    move(pos() + event->pos() - m_ConstPos);
  }
}

void WidgetBase::AddCloseButton()
{
  auto const button = new QPushButton(this);
  button->setFixedSize(14, 14);
  button->setStyleSheet(
    "QPushButton {"
      "background-color: #ea6e4d;"
      "border-radius: 7px;"
    "}"
    "QPushButton:hover {"
      "background-color: #ea6e4d;"
      "border-radius: 7px;"
      "border-image: url(:/icons/button-close.png);"
    "}"
  );
  button->setToolTip("关闭");
  connect(button, &QPushButton::clicked, this, &QWidget::close);
  m_Buttons.push_back(button);
}

void WidgetBase::AddMinButton()
{
  auto const button = new QPushButton(this);
  button->setFixedSize(14, 14);
  button->setStyleSheet(
    "QPushButton {"
      "background-color: #85c43b;"
      "border-radius: 7px;"
    "}"
    "QPushButton:hover {"
      "background-color: #85c43b;"
      "border-radius: 7px;"
      "border-image: url(:/icons/button-min.png);"
    "}"
  );
  button->setToolTip("最小化");
  connect(button, &QPushButton::clicked, this, &QWidget::showMinimized);
  m_Buttons.push_back(button);
}

