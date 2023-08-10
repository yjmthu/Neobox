#include <systemapi.h>
#include <trayframe.h>
#include <pluginmgr.h>
#include <speedbox.h>

#include <thread>
#include <chrono>

#include <QTimer>

using namespace std::literals;

TrayFrame::TrayFrame(SpeedBox& box)
#ifdef _WIN32
  : m_hReBar(nullptr)
  , m_hMin(nullptr)
  , m_hTaskBar(nullptr)
  , m_hNotify(nullptr)
  , m_Timer(new QTimer)
#else
  : m_Timer(new QTimer)
#endif
  , m_SpeedBox(box)
{
  m_Timer->setInterval(100);
  QObject::connect(m_Timer, &QTimer::timeout, std::bind(&TrayFrame::AdjustPosition, this));
  GetShellAllWnd();
  m_Timer->start();
}

TrayFrame::~TrayFrame()
{
  delete m_Timer;
}

void TrayFrame::GetShellAllWnd()
{
#ifdef _WIN32
  m_hTaskBar = FindWindowW(L"Shell_TrayWnd", nullptr);

  m_IsWin11TaskBar = (::FindWindowExW(m_hTaskBar, nullptr, L"Windows.UI.Composition.DesktopWindowContentBridge", nullptr) != nullptr);
  // 二级容器的句柄
  m_hReBar = ::FindWindowExW(m_hTaskBar, nullptr, L"ReBarWindow32", nullptr);
  m_hNotify = ::FindWindowExW(m_hTaskBar, nullptr, L"TrayNotifyWnd", nullptr);
  if (m_hReBar) {
    // 最小化窗口的句柄
    m_hMin = ::FindWindowExW(m_hReBar, nullptr, L"MSTaskSwWClass", nullptr);
  } else {
    mgr->ShowMsg("没有找到任务栏二级窗口！");
  }
#endif
}

#if 0
void TrayFrame::JoinInTray()
{
  if (!::IsWindow(m_hReBar)) {
    mgr->ShowMsg("任务栏图标句柄寻找失败！");
    return;
  }
  auto id = m_IsWin11TaskBar ? m_hTaskBar :m_hReBar;
  ::GetWindowRect(m_hMin, &m_rcMin);           //获得最小化窗口的区域
  ::GetWindowRect(m_hReBar, &m_rcReBar);           //获得二级容器的区域
  ::GetWindowRect(m_hTaskBar, &m_rcTaskBar);   //获得任务栏的矩形区域
  ::GetWindowRect(m_hNotify, &m_rcNotify);
  
  m_LeftSpace = m_rcMin.left - m_rcReBar.left;
  m_TopSpace = m_rcMin.top - m_rcReBar.top;
  //根据已经确定的任务栏最小化窗口区域得到屏幕并获得所在屏幕的DPI（Windows 8.1及其以上）
}

#endif

void TrayFrame::AdjustPosition()
{
  m_SpeedBox.raise();
}
