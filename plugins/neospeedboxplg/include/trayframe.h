#ifndef TRAY_FRAME_H
#define TRAY_FRAME_H

#ifdef _WIN32
#include <Windows.h>
#endif
#include <string>

class TrayFrame
{
public:
  typedef std::wstring String;

public:
  explicit TrayFrame(class SpeedBox&);
  ~TrayFrame();

private:
  void GetShellAllWnd();
  void AdjustPosition();

private:
  bool m_IsWin11TaskBar = false;
  int m_LeftSpace = 0;
  int m_TopSpace = 0;
  class QTimer* m_Timer;
  SpeedBox& m_SpeedBox;
#ifdef _WIN32
  // 任务栏
  HWND m_hTaskBar;
  // 任务栏子窗口
  HWND m_hReBar;
  HWND m_hMin;
  HWND m_hNotify;
  tagRECT m_rcTaskBar;
  tagRECT m_rcReBar;
  tagRECT m_rcMin;
  tagRECT m_rcNotify;
#endif
};

#endif // TRAY_FRAME_H