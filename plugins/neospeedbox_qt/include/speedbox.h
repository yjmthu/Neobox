#ifndef SPEEDBOX_H
#define SPEEDBOX_H

#include <QWidget>

class SpeedBox : public QWidget {
 private:
  QPoint m_ConstPos;
  class YJson& m_Settings;
  class PluginObject* m_PluginObject;
  class QWidget* m_CentralWidget;
  class NetSpeedHelper* m_NetSpeedHelper;
  class QLabel* m_TextMemUseage;
  class QLabel* m_TextUploadSpeed;
  class QLabel* m_TextDownloadSpeed;
  class QLabel* m_TextCpuUseage;
  class QFrame* m_MemColorFrame;
  class QTimer* m_Timer;
  class QMenu& m_NetCardMenu;

  std::string m_MemFrameStyle;

  class QPropertyAnimation* m_Animation;
  enum HideSide: int32_t {
    Left = 8, Right = 2,
    Top = 1, Bottom = 4,
    None = 0
  } m_HideSide = HideSide::None;

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  bool nativeEvent(const QByteArray& eventType,
                   void* message,
                   qintptr* result) override;
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;

 public:
  explicit SpeedBox(PluginObject* plugin, YJson& settings, QMenu* netcardMenu);
  ~SpeedBox();
  void InitShow();
  void InitMove();
  void UpdateSkin();

 private:
  void SetWindowMode();
  void SetBaseLayout();
  void UpdateTextContent();
  void SetHideFullScreen();
  void InitNetCard();
  void UpdateNetCardMenu();
  void UpdateNetCard(QAction* action, bool checked);
};

#endif
