#ifndef TRAY_FRAME_H
#define TRAY_FRAME_H

#include <QWidget>
#include <Windows.h>
#include <string>

class TrayFrame: public QWidget
{
public:
  typedef std::wstring String;

public:
  explicit TrayFrame();
  ~TrayFrame();

private:
  void GetShellAllWnd();
  void JoinInTray();
  void SetupUi();

private:
  // 任务栏
  HWND m_hTaskBar;
  // 任务栏子窗口
  HWND m_hReBar;
  HWND m_hTaskBand;
};

#endif // TRAY_FRAME_H