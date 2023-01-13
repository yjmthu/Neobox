#include <widgetbase.hpp>

#include <QPushButton>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QWindow>
#include <QScreen>

#include <Windows.h>
#include <Windowsx.h>

WidgetBase::WidgetBase(QWidget* parent, bool resizeAble)
  : QWidget(parent, Qt::FramelessWindowHint)
  , m_ResizeAble(resizeAble)
{
  setWindowIcon(QIcon(QStringLiteral(":/icons/neobox.ico")));
}

WidgetBase::~WidgetBase()
{
  //
}


void WidgetBase::showEvent(QShowEvent *event)
{
  QPoint point(width() - 35, 20);
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
  if(event->buttons() == Qt::LeftButton) {
    move(this->pos() + event->pos() - m_ConstPos);
    event->accept();
  }
  QWidget::mouseMoveEvent(event);
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

void WidgetBase::AddTitle(QString title)
{
  auto const label = new QLabel(title, this);
  label->move(20, 12);
}

void WidgetBase::SetShadowAround(QWidget* widget, int radius)
{
  auto const effect = new QGraphicsDropShadowEffect(this);
  effect->setOffset(0, 0);
  effect->setColor(Qt::gray);
  effect->setBlurRadius(radius);
  widget->setGraphicsEffect(effect);
}

static constexpr int padding = 11;

bool WidgetBase::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
  if (!m_ResizeAble) return false;

  MSG* msg = (MSG*)message;   
  switch(msg->message)
  {
  case WM_NCHITTEST:
    int xPos = GET_X_LPARAM(msg->lParam);
    int yPos = GET_Y_LPARAM(msg->lParam);
    QWindow * handle = window()->windowHandle();
    QScreen * screen = nullptr;
    if(handle && (screen = handle->screen())) {
      QScreen * screen = handle->screen();
      QPoint offset = screen->geometry().topLeft();
      xPos = (xPos - offset.x()) / screen->devicePixelRatio() + offset.x() - this->frameGeometry().x();
      yPos = (yPos - offset.y()) / screen->devicePixelRatio() + offset.y() - this->frameGeometry().y();
    } else {
      return false;
    }
    if(xPos < padding && yPos < padding)
      *result = HTTOPLEFT;
    else if(xPos >= width() - padding && yPos < padding)
      *result = HTTOPRIGHT;
    else if(xPos < padding && yPos >= height() - padding)
      *result = HTBOTTOMLEFT;
    else if(xPos >= width() - padding && yPos >= height() - padding)
      *result = HTBOTTOMRIGHT;
    else if(xPos < padding)
      *result =  HTLEFT;
    else if(xPos >= width() - padding)
      *result = HTRIGHT;
    else if(yPos < padding)
      *result = HTTOP;
    else if(yPos >= height() - padding)
      *result = HTBOTTOM;
    else
        break;
    return true;
  }
  return false;
}
