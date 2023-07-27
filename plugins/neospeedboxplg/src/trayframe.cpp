#include <systemapi.h>
#include <trayframe.h>
#include <pluginmgr.h>

#include <thread>
#include <chrono>

#include <QWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <Windows.h>

using namespace std::literals;

TrayFrame::TrayFrame()
  : QWidget(nullptr, Qt::FramelessWindowHint)
  , m_hReBar(nullptr)
  , m_hTaskBand(nullptr)
  , m_hTaskBar(nullptr)
{
  GetShellAllWnd();
  JoinInTray();
  SetupUi();
}

TrayFrame::~TrayFrame()
{
  setParent(nullptr);
}

void TrayFrame::GetShellAllWnd()
{
  m_hTaskBar = FindWindowW(L"Shell_TrayWnd", nullptr);
  while (!IsWindow(m_hTaskBar)) {
    if (!m_hTaskBar)
      std::this_thread::sleep_for(100ms);
  }
  m_hReBar = FindWindowExW(m_hTaskBar, nullptr, L"TaskbarAlphabeticalMFUContainer", nullptr);
  if (!m_hReBar)
    m_hReBar = FindWindowExW(m_hTaskBar, nullptr, L"ReBarWindow32", nullptr);
  if (m_hReBar) {
    m_hTaskBand = FindWindowExW(m_hReBar, nullptr, L"MSTaskSwWClass", nullptr);
  } else {
    mgr->ShowMsg("没有找到任务栏窗口！");
  }
}


void TrayFrame::JoinInTray()
{
  if (!m_hReBar) {
    // mgr->ShowMsg("任务栏图标句柄寻找失败！");
    return;
  }
  auto id = reinterpret_cast<WId>(m_hReBar);
  tagRECT rectReBar;
  if (!GetWindowRect(m_hReBar, &rectReBar)) {
    mgr->ShowMsg("获取任务栏窗口大小失败！");
    return;
  }
  MoveWindow(m_hTaskBand, 0, 0, rectReBar.right - rectReBar.left - 200, rectReBar.bottom - rectReBar.top, true);
  // auto const window = QWindow::fromWinId(id);
  // auto widget = QWidget::createWindowContainer(window);
  // if (!widget) {
  //   mgr->ShowMsg("从任务栏创建窗口失败！");
  //   return;
  // }
  move(918, 2);
  setFixedSize(100, 30);
  SetParent(reinterpret_cast<HWND>(winId()),
    reinterpret_cast<HWND>(id));
}

void TrayFrame::SetupUi()
{
  auto layout = new QHBoxLayout(this);
  auto label = new QLabel("上传: 0.0 B/s", this);
  layout->addWidget(label);
  label = new QLabel("下载: 0.0 B/s", this);
  layout->addWidget(label);
}

#if 0
INT_PTR CALLBACK TimeProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)//任务栏信息窗口过程
{
  switch (message)
  {
  case WM_INITDIALOG:
    return (INT_PTR)TRUE;
  case WM_ERASEBKGND:
  {
    //    PAINTSTRUCT ps;
    HDC hdc = (HDC)wParam;//BeginPaint(hDlg, &ps);    
    RECT rc;
    GetClientRect(hDlg, &rc);
    HDC mdc = CreateCompatibleDC(hdc);
    if (mdc) {
      HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
      HBITMAP oldBmp = (HBITMAP)SelectObject(mdc, hMemBmp);

      COLORREF rgb(RGB(255, 255, 255));
      COLORREF cBack = RGB(0, 0, 1);
      if (bThemeMode != 0) {
        rgb = RGB(8, 8, 8);
        //      if(rovi.dwBuildNumber>22000)
        //        cBack = RGB(254, 254, 255);
      }
      if (hWin11UI) {
        HBRUSH hb = CreateSolidBrush(cBack);
        FillRect(mdc, &rc, hb);
        DeleteObject(hb);
      }
      SYSTEMTIME systm;
      GetLocalTime(&systm);
      WCHAR sz[16];
      TCHAR szWeek[7][2] = { L"日",L"一",L"二",L"三",L"四",L"五",L"六" };

      int fsize;
      if (hWin11UI) {
        fsize = DPI(-11);
        wsprintf(sz, L"%.2d'%s", systm.wSecond, szWeek[systm.wDayOfWeek]);
      }
      else
      {
        fsize = DPI(-12);
        wsprintf(sz, L"%s%.2d:%.2d:%.2d", szWeek[systm.wDayOfWeek], systm.wHour, systm.wMinute, systm.wSecond);
      }
      HFONT hFont = CreateFont(fsize, 0, 0, 0, 0, false, false, false,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH, L"微软雅黑");
      HFONT oldFont = (HFONT)SelectObject(mdc, hFont);
      SetBkMode(mdc, TRANSPARENT);
      SetTextColor(mdc, rgb);
      if (!hWin11UI)
        DrawText(mdc, sz, lstrlen(sz), &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
      else
        DrawText(mdc, sz, 4, &rc, DT_LEFT | DT_SINGLELINE | DT_BOTTOM);
      SelectObject(mdc, oldFont);
      DeleteObject(hFont);

      if (!hWin11UI) {
        BYTE* lpvBits = NULL;
        BITMAPINFO binfo;
        memset(&binfo, 0, sizeof(BITMAPINFO));
        binfo.bmiHeader.biBitCount = 32;     //每个像素多少位，也可直接写24(RGB)或者32(RGBA)
        binfo.bmiHeader.biCompression = 0;
        binfo.bmiHeader.biHeight = rc.bottom - rc.top;
        binfo.bmiHeader.biPlanes = 1;
        binfo.bmiHeader.biSizeImage = (rc.bottom - rc.top) * (rc.right - rc.left) * 4;
        binfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        binfo.bmiHeader.biWidth = rc.right - rc.left;
        lpvBits = (BYTE*)HeapAlloc(GetProcessHeap(), NULL, binfo.bmiHeader.biSizeImage);
        //    GetDIBits(mdc, hMemBmp, 0, rc.bottom - rc.top, lpvBits, &bmpInfo, DIB_RGB_COLORS);
        if (lpvBits)
        {
          GetDIBits(mdc, hMemBmp, 0, rc.bottom - rc.top, lpvBits, &binfo, DIB_RGB_COLORS);
          for (DWORD i = 0; i < binfo.bmiHeader.biSizeImage - 4; i += 4)
          {
            if (lpvBits[i] > 3 || lpvBits[i + 1] != 0 || lpvBits[i + 2] != 0)
              lpvBits[i + 3] = 0x80;
          }
          SetDIBits(mdc, hMemBmp, 0, rc.bottom - rc.top, lpvBits, &binfo, DIB_RGB_COLORS);
          HeapFree(GetProcessHeap(), 0, lpvBits);
        }
      }
      BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, mdc, 0, 0, SRCCOPY);
      SelectObject(mdc, oldBmp);
      DeleteObject(hMemBmp);
      DeleteDC(mdc);
    }
    return TRUE;
  }
  break;
  }
  return FALSE;
}

#endif
