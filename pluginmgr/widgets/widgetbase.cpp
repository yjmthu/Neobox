#include <widgetbase.hpp>

#include <QPushButton>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QScrollBar>
#include <QFile>
#include <QMessageBox>
#include <QStyle>

#ifdef _WIN32
#include <Windows.h>
#include <Windowsx.h>
#else
#include <X11/Xlib.h>
#endif

static QString ReadStyle(QString path) {
  QFile file(path);
  file.open(QFile::ReadOnly);
  QString res = file.readAll();
  file.close();
  return res;
}

WidgetBase::WidgetBase(QWidget* parent, bool resizeAble, bool stayTop)
  : QWidget(parent, Qt::FramelessWindowHint)
  , m_ResizeAble(resizeAble)
  , m_StayTop(stayTop)
{
  setWindowIcon(QIcon(QStringLiteral(":/icons/neobox.ico")));
  setWindowFlag(Qt::WindowStaysOnTopHint, m_StayTop);
#ifdef __linux__
  // setWindowFlag(Qt::X11BypassWindowManagerHint);
  if (resizeAble) {
    setAttribute(Qt::WA_Hover, true);
    installEventFilter(this);
  }
#endif
}

WidgetBase::~WidgetBase()
{
  //
}


void WidgetBase::showEvent(QShowEvent *event)
{
  // UpdateBorderRect();
  if (m_ResizeAble) setMouseTracking(true);

  QWidget::showEvent(event);
}

void WidgetBase::hideEvent(QHideEvent *event)
{
  if (m_ResizeAble) setMouseTracking(false);
}

void WidgetBase::mousePressEvent(QMouseEvent* event)
{
  // https://gitee.com/feiyangqingyun/QWidgetDemo/blob/master/widget/framelesswidget/framelesscore/framelesswidget.cpp
  if (event->button() == Qt::LeftButton) {
    mouseRect = geometry();
    m_ConstPos = event->pos();
    if (pressedRect[0].contains(m_ConstPos)) {
      pressedArea |= 1;
    } else if (pressedRect[1].contains(m_ConstPos)) {
      pressedArea |= 1 << 1;
    } else if (pressedRect[2].contains(m_ConstPos)) {
      pressedArea |= 1 << 2;
    } else if (pressedRect[3].contains(m_ConstPos)) {
      pressedArea |= 1 << 3;
    } else if (pressedRect[4].contains(m_ConstPos)) {
      pressedArea |= 1 << 4;
    } else if (pressedRect[5].contains(m_ConstPos)) {
      pressedArea |= 1 << 5;
    } else if (pressedRect[6].contains(m_ConstPos)) {
      pressedArea |= 1 << 6;
    } else if (pressedRect[7].contains(m_ConstPos)) {
      pressedArea |= 1 << 7;
    } else {
      pressedArea = 0;
    }

  }

}

void WidgetBase::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    // if (hasMouseTracking()) {
    //   setMouseTracking(false);
    // }
    pressedArea = 0;
    m_ConstPos = QPoint();
  }
}

void WidgetBase::mouseMoveEvent(QMouseEvent* event)
{
  if(!m_ResizeAble && event->buttons() == Qt::LeftButton && !m_ConstPos.isNull()) {
    move(event->globalPosition().toPoint() - m_ConstPos);
    event->accept();
    return;
  }

  if (m_ConstPos.isNull()) return;

  QPoint point = event->position().toPoint();
  //根据当前鼠标位置,计算 X Y 轴移动了多少
  int offsetX = point.x() - m_ConstPos.x();
  int offsetY = point.y() - m_ConstPos.y();

  //根据按下处的位置判断是否是移动控件还是拉伸控件
  if (!pressedArea) {
    this->move(this->x() + offsetX, this->y() + offsetY);
  }

  int rectX = mouseRect.x();
  int rectY = mouseRect.y();
  int rectW = mouseRect.width();
  int rectH = mouseRect.height();

  if (pressedArea & 1) {
    int resizeW = this->width() - offsetX;
    if (this->minimumWidth() <= resizeW) {
      this->setGeometry(this->x() + offsetX, rectY, resizeW, rectH);
    }
  } else if ((pressedArea >> 1) & 1) {
    this->setGeometry(rectX, rectY, rectW + offsetX, rectH);
  } else if ((pressedArea >> 2) & 1) {
    int resizeH = this->height() - offsetY;
    if (this->minimumHeight() <= resizeH) {
      this->setGeometry(rectX, this->y() + offsetY, rectW, resizeH);
    }
  } else if ((pressedArea >> 3) & 1) {
    this->setGeometry(rectX, rectY, rectW, rectH + offsetY);
  } else if ((pressedArea >> 4) & 1) {
    int resizeW = this->width() - offsetX;
    int resizeH = this->height() - offsetY;
    if (this->minimumWidth() <= resizeW) {
      this->setGeometry(this->x() + offsetX, this->y(), resizeW, resizeH);
    }
    if (this->minimumHeight() <= resizeH) {
      this->setGeometry(this->x(), this->y() + offsetY, resizeW, resizeH);
    }
  } else if ((pressedArea >> 5) & 1) {
    int resizeW = rectW + offsetX;
    int resizeH = this->height() - offsetY;
    if (this->minimumHeight() <= resizeH) {
      this->setGeometry(this->x(), this->y() + offsetY, resizeW, resizeH);
    }
  } else if ((pressedArea >> 6) & 1) {
    int resizeW = this->width() - offsetX;
    int resizeH = rectH + offsetY;
    if (this->minimumWidth() <= resizeW) {
      this->setGeometry(this->x() + offsetX, this->y(), resizeW, resizeH);
    }
    if (this->minimumHeight() <= resizeH) {
      this->setGeometry(this->x(), this->y(), resizeW, resizeH);
    }
  } else if ((pressedArea >> 7) & 1) {
    int resizeW = rectW + offsetX;
    int resizeH = rectH + offsetY;
    this->setGeometry(this->x(), this->y(), resizeW, resizeH);
  }
  QWidget::mouseMoveEvent(event);
}

void WidgetBase::resizeEvent(QResizeEvent* event) {
  UpdateBorderRect();

  QPoint point(width() - 35, 20);
  for (auto button: m_Buttons) {
    button->move(point);
    point.rx() -= 30;
  }

  event->accept();
}

static constexpr int padding = 11;

bool WidgetBase::eventFilter(QObject *watched, QEvent *event) {

  static const QString styleWide { ReadStyle(":/styles/ScrollbarWide.qss") };
  static const QString styleNarrow { ReadStyle(":/styles/ScrollbarNarrow.qss") };

  if (watched == this && m_ResizeAble && event->type() == QEvent::HoverMove) {
    QPoint point = reinterpret_cast<QHoverEvent*>(event)->position().toPoint();
    if (pressedRect[0].contains(point)) {
      setCursor(Qt::SizeHorCursor);
    } else if (pressedRect[1].contains(point)) {
      setCursor(Qt::SizeHorCursor);
    } else if (pressedRect[2].contains(point)) {
      setCursor(Qt::SizeVerCursor);
    } else if (pressedRect[3].contains(point)) {
      setCursor(Qt::SizeVerCursor);
    } else if (pressedRect[4].contains(point)) {
      setCursor(Qt::SizeFDiagCursor);
    } else if (pressedRect[5].contains(point)) {
      setCursor(Qt::SizeBDiagCursor);
    } else if (pressedRect[6].contains(point)) {
      setCursor(Qt::SizeBDiagCursor);
    } else if (pressedRect[7].contains(point)) {
      setCursor(Qt::SizeFDiagCursor);
    } else {
      setCursor(Qt::ArrowCursor);
    }
  } else {
    auto iter = m_ScrollBars.find(reinterpret_cast<QScrollBar *>(watched));
    if (iter != m_ScrollBars.end()) {
      auto const bar = *iter;
      if (event->type() == QEvent::HoverEnter)
      {
        bar->setFixedWidth(10); //重新定义宽度
        bar->setProperty("STYLE_KEY", QString("SETTINGSSWBG_SCROLL_HOVER")); //重载样式
        bar->setStyleSheet(styleWide);
        bar->style()->polish(bar); //强制刷新样式
      }
      else if (event->type() == QEvent::HoverLeave)
      {
        bar->setFixedWidth(4);
        bar->setProperty("STYLE_KEY", QString("SETTINGSSWBG_SCROLL"));
        bar->setStyleSheet(styleNarrow);
        bar->style()->polish(bar);
      }
    }
  }
  return QWidget::eventFilter(watched, event);
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

void WidgetBase::AddScrollBar(QScrollBar* bar)
{
  m_ScrollBars.insert(bar);
  bar->installEventFilter(this);
  bar->setStyleSheet(ReadStyle(":/styles/ScrollbarNarrow.qss"));
  bar->setFixedWidth(4);
}

void WidgetBase::RemoveScrollBar(QScrollBar* bar)
{
  m_ScrollBars.erase(bar);
  bar->removeEventFilter(bar);
}

void WidgetBase::SetShadowAround(QWidget* widget, int radius, QColor col, int dx, int dy)
{
  auto const effect = new QGraphicsDropShadowEffect(this);
  effect->setOffset(dx, dy);
  effect->setColor(col);
  effect->setBlurRadius(radius);
  widget->setGraphicsEffect(effect);
}

void WidgetBase::UpdateBorderRect()
{
  int width = this->width();
  int height = this->height();

  //左侧描点区域
  pressedRect[0] = QRect(0, padding, padding, height - padding * 2);
  //右侧描点区域
  pressedRect[1] = QRect(width - padding, padding, padding, height - padding * 2);
  //上侧描点区域
  pressedRect[2] = QRect(padding, 0, width - padding * 2, padding);
  //下侧描点区域
  pressedRect[3] = QRect(padding, height - padding, width - padding * 2, padding);

  //左上角描点区域
  pressedRect[4] = QRect(0, 0, padding, padding);
  //右上角描点区域
  pressedRect[5] = QRect(width - padding, 0, padding, padding);
  //左下角描点区域
  pressedRect[6] = QRect(0, height - padding, padding, padding);
  //右下角描点区域
  pressedRect[7] = QRect(width - padding, height - padding, padding, padding);
}

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
    // QWindow* window = QWindow::fromWinId((WId)QWidget::winId());
    // auto const pos = window->mapFromGlobal(QPoint(xPos, yPos));
    // xPos = pos.x(), yPos = pos.y();
    QWindow * handle = window()->windowHandle();
    if (QScreen * screen = nullptr; handle && (screen = handle->screen())) {
      xPos = xPos / screen->devicePixelRatio();
      yPos = yPos / screen->devicePixelRatio();
      auto const pos = handle->mapFromGlobal(QPoint(xPos, yPos));
      xPos = pos.x(), yPos = pos.y();
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
