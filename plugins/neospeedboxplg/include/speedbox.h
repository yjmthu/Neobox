#ifndef SPEEDBOX_H
#define SPEEDBOX_H

#include <widgetbase.hpp>

#include <filesystem>
#include <pluginobject.h>
#include <speedboxcfg.h>

namespace fs = std::filesystem;

class SpeedBox : public WidgetBase {
private:
  SpeedBoxCfg& m_Settings;
  class NeoSpeedboxPlg* m_PluginObject;
  class SkinObject* m_CentralWidget;
#ifdef _WIN32
  HINSTANCE m_SkinDll;
#else
  void* m_SkinDll;
#endif
  class QTimer* m_Timer;
  class TrayFrame* m_TrayFrame;
  class MenuBase& m_NetCardMenu;
  void* m_AppBarData;
  class NetSpeedHelper& m_NetSpeedHelper;
  class ProcessForm* m_ProcessForm;
  std::string m_MemFrameStyle;

  class QPropertyAnimation* m_Animation;
  enum HideSide: int32_t {
    Left = 8, Right = 2,
    Top = 1, Bottom = 4,
    None = 0
  } m_HideSide = HideSide::None;

protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
#ifdef _WIN32
  bool nativeEvent(const QByteArray& eventType,
                   void* message,
                   qintptr* result) override;
#endif
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;

 public:
  explicit SpeedBox(NeoSpeedboxPlg* plugin, SpeedBoxCfg& settings, MenuBase* netcardMenu);
  ~SpeedBox();
  void InitShow(const PluginObject::FollowerFunction& callback);
  void InitMove();
  void UpdateSkin();
  void SetProgressMonitor(bool on);
  void SetTrayMode(bool on);

 private:
  void SetWindowMode();
  bool LoadDll(fs::path dllPath);
  void LoadCurrentSkin();
#ifdef _WIN32
  void SetHideFullScreen();
#endif
  void InitNetCard();
  void UpdateNetCardMenu();
  void UpdateNetCard(QAction* action, bool checked);
};

#endif
