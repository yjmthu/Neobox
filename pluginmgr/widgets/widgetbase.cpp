#include <widgetbase.hpp>

#include <QPushButton>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QWindow>
#include <QScreen>

#ifdef _WIN32
#include <Windows.h>
#include <Windowsx.h>
#else
#endif

WidgetBase::WidgetBase(QWidget* parent, bool resizeAble, bool stayTop)
  : QWidget(parent, Qt::FramelessWindowHint)
  , m_ResizeAble(resizeAble)
  , m_StayTop(stayTop)
{
  setWindowIcon(QIcon(QStringLiteral(":/icons/neobox.ico")));
  setWindowFlag(Qt::WindowStaysOnTopHint, m_StayTop);
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
  QWidget::showEvent(event);
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
    m_ConstPos = QPoint();
  }
}

void WidgetBase::mouseMoveEvent(QMouseEvent* event)
{
  if(event->buttons() == Qt::LeftButton && !m_ConstPos.isNull()) {
    move(event->globalPosition().toPoint() - m_ConstPos);
    event->accept();
  }
  QWidget::mouseMoveEvent(event);
}

void WidgetBase::AddTopButton()
{
  auto const button = new QPushButton(this);
  button->setFixedSize(14, 14);
  button->setCheckable(true);
  button->setChecked(m_StayTop);
  button->setStyleSheet(
    "QPushButton {"
      "background-color: #009999;"
      "border-radius: 7px;"
    "}"
    "QPushButton:hover {"
      "background-color: #00aaaa;"
      "border-radius: 7px;"
      "border-image: url(:/icons/button-top.png);"
    "}"
    "QPushButton:checked {"
      "background-color: #00dddd;"
      "border-radius: 7px;"
      // "border-image: url(:/icons/button-top.png);"
    "}"
  );
  button->setToolTip("置顶");
  connect(button, &QPushButton::clicked, this, [this](bool on){
    setWindowFlag(Qt::WindowStaysOnTopHint, on);
    m_StayTop = on;
    show();
    SaveTopState(on);
  });
  m_Buttons.push_back(button);
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

void WidgetBase::SetShadowAround(QWidget* widget, int radius, QColor col, int dx, int dy)
{
  auto const effect = new QGraphicsDropShadowEffect(this);
  effect->setOffset(dx, dy);
  effect->setColor(col);
  effect->setBlurRadius(radius);
  widget->setGraphicsEffect(effect);
}

static constexpr int padding = 11;

bool WidgetBase::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
  if (!m_ResizeAble) return false;
#ifdef _WIN32
  MSG* msg = (MSG*)message;   
  switch(msg->message)
  {
  case WM_NCHITTEST:
    int xPos = GET_X_LPARAM(msg->lParam);
    int yPos = GET_Y_LPARAM(msg->lParam);
    QWindow * handle = window()->windowHandle();
    if (QScreen * screen = nullptr; handle && (screen = handle->screen())) {
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
#endif
  return false;
}
