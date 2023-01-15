#include <smallform.hpp>
#include <squareform.hpp>
#include <ui_smallform.h>
#include <pluginmgr.h>

#include <QWindow>
#include <QScreen>

#include <Windows.h>

SmallForm::SmallForm(QWidget* parent)
  : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
  , m_Color(Qt::white)
  , ui(new Ui::SmallForm)
  , m_Screen(nullptr)
  , m_SquareForm(nullptr)
  , m_ScaleTimes(0)
{
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi(this);
}

SmallForm::~SmallForm()
{
  if (m_SquareForm) {
    delete m_SquareForm;
  }
  delete ui;
}

void SmallForm::showEvent(QShowEvent *event)
{
  QWindow * handle = window()->windowHandle();
  if(handle) {
    m_Screen = handle->screen();
  }
  AutoPosition();
  QWidget::showEvent(event);
}

void SmallForm::GetScreenColor(int x, int y)
{
  HWND hWnd = ::GetDesktopWindow();
  HDC hdc = ::GetDC(hWnd); // GetDC(NULL)
  COLORREF pixel = ::GetPixel(hdc, x, y);
  if (pixel != CLR_INVALID) {
    SetColor(QColor(
      GetRValue(pixel),
      GetGValue(pixel),
      GetBValue(pixel)
    ));
  }
}

void SmallForm::SetColor(const QColor& color)
{
  m_Color = color;

  auto text = m_Color.name(QColor::NameFormat::HexRgb);
  ui->frame->setStyleSheet(QStringLiteral("background-color:%1;").arg(text));
  ui->label->setText(text);
}

void SmallForm::TransformPoint()
{
  if(m_Screen) {
    QPoint offset = m_Screen->geometry().topLeft();
    m_Position = (m_Position - offset) / m_Screen->devicePixelRatio() + offset;
  } else {
    m_Position = QPoint(0, 0);
  }
}

void SmallForm::AutoPosition()
{
  if (!m_Screen) return;
  TransformPoint();

  constexpr int delta = 20;
  auto frame = frameGeometry();
  QPoint destination = frame.topLeft();
  auto sizeScreen = m_Screen->size();

  if (m_Position.y() + delta + frame.height() >= sizeScreen.height()) {
    destination.ry() = m_Position.y() - delta - frame.height();
  } else {
    destination.ry() = m_Position.y() + delta;
  }

  if (m_Position.x() + delta + frame.width() >= sizeScreen.width()) {
    destination.rx() = m_Position.x() - delta - frame.width();
  } else {
    destination.rx() = m_Position.x() + delta;
  }
  move(destination);

  raise();
}

void SmallForm::MouseWheel(short value)
{
  constexpr int delta = 20;
  if (!m_Screen) {
    mgr->ShowMsg("获取当前屏幕失败！");
    return;
  }
  if (value > 0) {
    if (!m_SquareForm) {
      m_SquareForm = new SquareForm(m_Screen->grabWindow(0, m_Position.x() - delta, m_Position.y() - delta, delta * 2, delta * 2), m_Position);
      m_SquareForm->show();
      m_ScaleTimes = 1;
      raise();
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
        delete m_SquareForm;
        m_SquareForm = nullptr;
      }
    }
  }
}