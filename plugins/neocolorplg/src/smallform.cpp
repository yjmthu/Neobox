#include <smallform.hpp>
#include <squareform.hpp>
#include <pluginmgr.h>
#include <colordlg.hpp>
#include <yjson.h>
#include <colorconfig.h>
#include <colorback.hpp>
#include <screenfetch.hpp>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWindow>
#include <QScreen>
#include <QPainter>
#include <QPainterPath>

SmallForm* SmallForm::m_Instance = nullptr;

SmallForm::SmallForm(ColorConfig& settings)
  : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
  , m_Settings(settings)
  , m_Color()
  , m_ScreenFetch(new ScreenFetch)
  , m_SquareForm(nullptr)
  , m_ScaleTimes(0)
  , m_ColorPath()
  , m_TextOption(Qt::AlignLeft | Qt::AlignVCenter)
  , m_TextFont("Microsoft YaHei UI", 14, 700)
{
  m_Instance = this;
  setAttribute(Qt::WA_TranslucentBackground);

  setFixedSize(150, 50);
  m_BackPixMap = QPixmap(size());
  m_BackPixMap.fill(Qt::transparent);
  m_ColorPath.addRoundedRect(QRect(0, 0, 34, 34), 8, 8);
  QPainter painter(&m_BackPixMap);
  painter.setRenderHint(QPainter::Antialiasing, true);
  QPainterPath path;
  path.addRoundedRect(rect(), 4, 4);
  painter.fillPath(path, QColor(50, 50, 50, 240));

  ConnectAll(m_ScreenFetch);
  setParent(m_ScreenFetch);
  m_ScreenFetch->showFullScreen();
}

SmallForm::~SmallForm()
{
  setParent(nullptr);
  delete m_SquareForm;
  m_Instance = nullptr;
  delete m_ScreenFetch;
}

void SmallForm::ConnectAll(class ColorBack* back)
{
  connect(back, &ScreenFetch::InitShow, this, [this]() {
    QPoint pos = QCursor::pos();
    pos -= m_ScreenFetch->pos();
    AutoPosition(pos);
    show();
    activateWindow();
  });
  connect(back, &ScreenFetch::MouseMove, this, [this](QPoint pos, QColor col) {
    pos -= m_ScreenFetch->pos();
    AutoPosition(pos);
    SetColor(col);
  });
  connect(back, &ScreenFetch::MouseClick, this, [this](QColor col) {
    if (col.isValid()) {
      SetColor(col);
      m_ScreenFetch->close();
      QuitHook(true);
    } else {
      m_ScreenFetch->close();
      QuitHook(false);
    }
  });
  connect(back, &ColorBack::MouseWheel, this, &SmallForm::OnMouseWheel);
}

void SmallForm::QuitHook(bool succeed)
{
  if (succeed) {
    if (!ColorDlg::m_Instance) {
      ColorDlg::m_Instance = new ColorDlg(m_Instance->m_Settings);
    }
    ColorDlg::m_Instance->AddColor(m_Instance->m_Color);
    ColorDlg::m_Instance->show();
  } else if (ColorDlg::m_Instance) {
    delete ColorDlg::m_Instance;
  }
  delete this;
}

void SmallForm::PickColor(ColorConfig& settings)
{
  if (m_Instance) {
    mgr->ShowMsg("正在拾取颜色！");
    return;
  }
  if (ColorDlg::m_Instance) {
    ColorDlg::m_Instance->hide();
  }

  m_Instance = new SmallForm(settings);
}

void SmallForm::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.drawPixmap(0, 0, m_BackPixMap);

  painter.translate(8, 8);
  painter.fillPath(m_ColorPath, m_Color);

  painter.setPen(Qt::white);
  painter.setFont(m_TextFont);
  painter.drawText(QRect(45, 0, 90, 34), m_Color.name(), m_TextOption);
}


void SmallForm::OnMouseWheel(QWheelEvent *event)
{
  const QPoint& numDegrees = event->angleDelta() / 120;
  const QPoint& point = event->globalPosition().toPoint();

  if (numDegrees.isNull() || point.isNull()) {
    mgr->ShowMsg("获取滚动信息失败！");
    event->accept();
    return;
  }

  constexpr int delta = 20;
  const int value = numDegrees.y();

  if (value > 0) {
    if (!m_SquareForm) {
      QPoint leftTop(point.x() - delta, point.y() - delta);
      QPoint rightBottom(delta * 2, delta * 2);
      m_ScreenFetch->TransformPoint(rightBottom);
      QRect rect(m_ScreenFetch->TransformPoint(leftTop), QSize(rightBottom.x(), rightBottom.ry()));
      m_SquareForm = new SquareForm(m_ScreenFetch->m_Pixmap.copy(rect), point);
      m_SquareForm->setParent(m_ScreenFetch);
      ConnectAll(m_SquareForm);
      m_SquareForm->show();
      m_ScaleTimes = 1;
      this->raise();
    } else {
      if (m_ScaleTimes + value <= 4) {
        m_ScaleTimes += value;
        m_SquareForm->SetScaleSize(m_ScaleTimes);
      }
    }
  } else if (value < 0) {
    if (m_ScaleTimes + value > 0) {
      m_ScaleTimes += value;
      m_SquareForm->SetScaleSize(m_ScaleTimes);
    } else {
      m_ScaleTimes = 0;
      if (m_SquareForm) {
        setParent(m_ScreenFetch);
        delete m_SquareForm;
        m_SquareForm = nullptr;
      }
    }
  }

  event->accept();
}

void SmallForm::SetColor(const QColor& color)
{
  if (m_Color == color) return;
  m_Color = color;
  update();
}

void SmallForm::AutoPosition(const QPoint& point)
{
  constexpr int delta = 20;
  auto frame = frameGeometry();
  QPoint destination = frame.topLeft();

  if (point.y() + delta + frame.height() >= m_ScreenFetch->height()) {
    destination.ry() = point.y() - delta - frame.height();
  } else {
    destination.ry() = point.y() + delta;
  }

  if (point.x() + delta + frame.width() >= m_ScreenFetch->width()) {
    destination.rx() = point.x() - delta - frame.width();
  } else {
    destination.rx() = point.x() + delta;
  }
  move(destination);
}
