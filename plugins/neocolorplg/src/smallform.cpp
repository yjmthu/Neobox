#include <smallform.hpp>
#include <squareform.hpp>
#include <pluginmgr.h>
#include <colordlg.hpp>
#include <yjson.h>
#include <colorconfig.h>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWindow>
#include <QScreen>
#include <QPainter>
#include <QPainterPath>

#include <Windows.h>
#include <Windowsx.h>

SmallForm* SmallForm::m_Instance = nullptr;

HHOOK SmallForm::m_Hoock[2] { nullptr, nullptr };

SmallForm::SmallForm(ColorConfig& settings)
  : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
  , m_Settings(settings)
  , m_Color()
  , m_Screen(nullptr)
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
}

SmallForm::~SmallForm()
{
  if (m_SquareForm) {
    delete m_SquareForm;
  }
  m_Instance = nullptr;
  if (!UninstallHook()) {
    mgr->ShowMsg(QStringLiteral("卸载钩子失败！\n错误代码【%1】").arg(GetLastError()));
  }
}

static constexpr auto WM_USER_MOUSEMOVE = WM_USER + 1;

LRESULT CALLBACK SmallForm::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (!m_Instance || nCode != HC_ACTION) CallNextHookEx(m_Hoock[0], nCode, wParam, lParam);

  auto const mouseHookStruct = (MSLLHOOKSTRUCT*)lParam;
  switch (wParam) {
  case WM_LBUTTONDOWN:
  case WM_MOUSEWHEEL:
		PostMessageW(
      reinterpret_cast<HWND>(m_Instance->winId()),
      wParam, mouseHookStruct->mouseData, MAKELPARAM(mouseHookStruct->pt.x, mouseHookStruct->pt.y)
    );
    return 1;
  case WM_MOUSEMOVE:
		PostMessageW(
      reinterpret_cast<HWND>(m_Instance->winId()),
      WM_USER_MOUSEMOVE, MK_CONTROL, MAKELPARAM(mouseHookStruct->pt.x, mouseHookStruct->pt.y)
    );
  default:
    break;
  }
	return CallNextHookEx(SmallForm::m_Hoock[0], nCode, wParam, lParam);
}

LRESULT CALLBACK SmallForm::LowLevelKeyProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (!m_Instance) CallNextHookEx(m_Hoock[1], nCode, wParam, lParam);

  switch (wParam) {
  case WM_KEYDOWN: {
		auto const keyHookStruct = (KBDLLHOOKSTRUCT*)lParam;
    if (keyHookStruct->vkCode != VK_ESCAPE || !m_Instance) break;
    // m_Instance->QuitHook(false);
		PostMessage(
      reinterpret_cast<HWND>(m_Instance->winId()),
      WM_KEYDOWN, VK_ESCAPE, keyHookStruct->scanCode
    );
    return 1;
	}
  default:
    break;
  }
	return CallNextHookEx(SmallForm::m_Hoock[1], nCode, wParam, lParam);
}

bool SmallForm::InstallHook()
{
  // https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowshookexw

  if (!m_Hoock[0]) {
    m_Hoock[0] = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandleW(NULL), 0);
  }
  if (!m_Hoock[1]) {
    m_Hoock[1] = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyProc, GetModuleHandleW(NULL), 0);
  }
  return m_Hoock[0] && m_Hoock[1];
}

bool SmallForm::UninstallHook()
{
  if (!m_Hoock[0] && !m_Hoock[1]) {
    return true;
  }
  if (!m_Hoock[0] || UnhookWindowsHookEx(m_Hoock[0])) {
    m_Hoock[0] = nullptr;
  }
  if (!m_Hoock[1] || UnhookWindowsHookEx(m_Hoock[1])) {
    m_Hoock[1] = nullptr;
  }
  return !(m_Hoock[0] || m_Hoock[1]);
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
  if (!m_Instance->InstallHook()) {
    mgr->ShowMsg(QStringLiteral("注册钩子失败！\n错误代码【%1】。").arg(GetLastError()));
    return;
  }
  POINT pt;
  if (!GetCursorPos(&pt)) {
    mgr->ShowMsg(QStringLiteral("获取鼠标位置失败！\n错误代码【%1】。").arg(GetLastError()));
    return;
  }

  if (ColorDlg::m_Instance) {
    ColorDlg::m_Instance->hide();
  }

  m_Instance = new SmallForm(settings);
  // m_Instance->m_Position = QPoint(pt.x, pt.y);
  m_Instance->show();
}

void SmallForm::showEvent(QShowEvent *event)
{
  QWindow * handle = window()->windowHandle();
  if(handle) {
    m_Screen = handle->screen();
    POINT pt;
    if (GetCursorPos(&pt)) {
      QPoint point(pt.x, pt.y);
      TransformPoint(point);
      move(point);
    }
  }
  QWidget::showEvent(event);
}

void SmallForm::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape) {
    QuitHook(false);
    event->accept();
  }
}

void SmallForm::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton) {
    QuitHook(true);
  }
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

bool SmallForm::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
  if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
  {
    MSG* pMsg = reinterpret_cast<MSG*>(message);
    switch (pMsg->message) {
    case WM_USER_MOUSEMOVE: {
      QPoint point(GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam));
      GetScreenColor(point.x(), point.y());
      TransformPoint(point);
      AutoPosition(point);
      if (m_SquareForm) {
        m_SquareForm->MouseMove(point);
      }
      return true;
    }
    default:
      break;
    }
  }
  return QWidget::nativeEvent(eventType, message, result);
}

void SmallForm::wheelEvent(QWheelEvent *event)
{
  const QPoint& numDegrees = event->angleDelta() / 120;
  const QPoint& point = event->globalPosition().toPoint();

  if (numDegrees.isNull() || point.isNull()) {
    mgr->ShowMsg("获取滚动信息失败！");
    event->accept();
    return;
  }

  if (!m_Screen) {
    mgr->ShowMsg("获取当前屏幕失败！");
    event->accept();
    return;
  }

  constexpr int delta = 20;
  const int value = numDegrees.y();

  if (value > 0) {
    if (!m_SquareForm) {
      m_SquareForm = new SquareForm(m_Screen->grabWindow(0, point.x() - delta, point.y() - delta, delta * 2, delta * 2), point);
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

  event->accept();
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
  if (m_Color == color) return;
  
  m_Color = color;
  update();

  // ui->frame->update();
  // auto text = m_Color.name(QColor::NameFormat::HexRgb);
  // ui->frame->setStyleSheet(QStringLiteral("background-color:%1;").arg(text));
}

void SmallForm::TransformPoint(QPoint& point)
{
  if(m_Screen) {
    QPoint offset = m_Screen->geometry().topLeft();
    point = (point - offset) / m_Screen->devicePixelRatio() + offset;
  } else {
    point = QPoint(0, 0);
  }
}

void SmallForm::AutoPosition(const QPoint& point)
{
  if (!m_Screen) return;
  // TransformPoint();

  constexpr int delta = 20;
  auto frame = frameGeometry();
  QPoint destination = frame.topLeft();
  auto sizeScreen = m_Screen->size();

  if (point.y() + delta + frame.height() >= sizeScreen.height()) {
    destination.ry() = point.y() - delta - frame.height();
  } else {
    destination.ry() = point.y() + delta;
  }

  if (point.x() + delta + frame.width() >= sizeScreen.width()) {
    destination.rx() = point.x() - delta - frame.width();
  } else {
    destination.rx() = point.x() + delta;
  }
  move(destination);

}
