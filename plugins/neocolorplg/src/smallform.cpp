#include <smallform.hpp>
#include <squareform.hpp>
#include <ui_smallform.h>
#include <pluginmgr.h>
#include <colordlg.hpp>
#include <yjson.h>

#include <QWindow>
#include <QScreen>

#include <Windows.h>

SmallForm* SmallForm::m_Instance = nullptr;

HHOOK SmallForm::m_Hoock[2] { nullptr, nullptr };

SmallForm::SmallForm(YJson& settings)
  : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool)
  , m_Settings(settings)
  , m_Color(Qt::white)
  , ui(new Ui::SmallForm)
  , m_Screen(nullptr)
  , m_SquareForm(nullptr)
  , m_ScaleTimes(0)
{
  m_Instance = this;
  setAttribute(Qt::WA_TranslucentBackground);
  ui->setupUi(this);
}

SmallForm::~SmallForm()
{
  if (m_SquareForm) {
    delete m_SquareForm;
  }
  delete ui;
  m_Instance = nullptr;
  if (!UninstallHook()) {
    mgr->ShowMsg(QStringLiteral("卸载钩子失败！\n错误代码【%1】").arg(GetLastError()));
  }
}

LRESULT CALLBACK SmallForm::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (!m_Instance) CallNextHookEx(SmallForm::m_Hoock[0], nCode, wParam, lParam);

  switch (wParam) {
  case WM_LBUTTONDOWN:
    // m_Instance->DoMouseMove(lParam);
    m_Instance->QuitHook(true);
    return 1;
  case WM_MOUSEWHEEL:
    m_Instance->DoMouseWheel(lParam);
    return 1;
  case WM_MOUSEMOVE:
    m_Instance->DoMouseMove(lParam);
    break;
		// PostMessage(
    //   reinterpret_cast<HWND>(ColorDlg::m_Instance->winId()),
    //   wParam, MK_CONTROL, MAKELPARAM(pt.x, pt.y)
    // );
    // break;
  case WM_KEYDOWN: {
		auto const keyHookStruct = (KBDLLHOOKSTRUCT*)lParam;
    if (keyHookStruct->vkCode != VK_ESCAPE) break;
    m_Instance->QuitHook(false);
		// PostMessage(
    //   reinterpret_cast<HWND>(ColorDlg::m_Instance->winId()),
    //   WM_KEYDOWN, VK_ESCAPE, keyHookStruct->scanCode
    // );
    return 1;
	}
  default:
    break;
  }
	return CallNextHookEx(SmallForm::m_Hoock[0], nCode, wParam, lParam);
}

LRESULT CALLBACK SmallForm::LowLevelKeyProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (!m_Instance) CallNextHookEx(SmallForm::m_Hoock[1], nCode, wParam, lParam);

  switch (wParam) {
  case WM_KEYDOWN: {
		auto const keyHookStruct = (KBDLLHOOKSTRUCT*)lParam;
    if (keyHookStruct->vkCode != VK_ESCAPE) break;
    m_Instance->QuitHook(false);
		// PostMessage(
    //   reinterpret_cast<HWND>(ColorDlg::m_Instance->winId()),
    //   WM_KEYDOWN, VK_ESCAPE, keyHookStruct->scanCode
    // );
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

void SmallForm::DoMouseMove(LPARAM lParam)
{
  if (!m_Instance) {
    mgr->ShowMsg("失败，未能获取颜色悬浮窗！");
    return;
  }
  auto const mouseHookStruct = (MSLLHOOKSTRUCT*)lParam;
  POINT pt = mouseHookStruct->pt;
  m_Position = QPoint(pt.x, pt.y);
  GetScreenColor(pt.x, pt.y);
  AutoPosition();
}

void SmallForm::DoMouseWheel(LPARAM lParam)
{
  if (!m_Instance) {
    mgr->ShowMsg("失败，未能获取颜色悬浮窗！");
    return;
  }
  auto const mouseHookStruct = (MSLLHOOKSTRUCT*)lParam;
  POINT pt = mouseHookStruct->pt;
  auto zDelta = GET_WHEEL_DELTA_WPARAM(mouseHookStruct->mouseData);
  m_Position = QPoint(pt.x, pt.y);
  GetScreenColor(pt.x, pt.y);
  AutoPosition();
  MouseWheel(zDelta / WHEEL_DELTA);
}

void SmallForm::QuitHook(bool succeed)
{
  if (m_Instance) {
    if (succeed) {
      if (!ColorDlg::m_Instance) {
        ColorDlg::m_Instance = new ColorDlg(m_Instance->m_Settings);
      }
      ColorDlg::m_Instance->AddColor(m_Instance->m_Color);
      ColorDlg::m_Instance->show();
    } else if (ColorDlg::m_Instance) {
      delete ColorDlg::m_Instance;
    }
    delete m_Instance;
  }
}

void SmallForm::PickColor(YJson& settings)
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
  m_Instance->m_Position = QPoint(pt.x, pt.y);
  m_Instance->show();
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