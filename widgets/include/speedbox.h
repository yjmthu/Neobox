#ifndef SPEEDBOX_H
#define SPEEDBOX_H

#include <QWidget>

#include <netspeedhelper.h>

class SpeedBox : public QWidget {
 private:
  QPoint m_ConstPos;
  class QWidget* m_CentralWidget;
  NetSpeedHelper m_NetSpeedHelper;
  class QLabel* m_TextMemUseage;
  class QLabel* m_TextUploadSpeed;
  class QLabel* m_TextDownLoadSpeed;
  class QTimer* m_Timer;
  class NeoMenu* m_MainMenu;

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;

 public:
  explicit SpeedBox(QWidget* parent = nullptr);
  ~SpeedBox();

 private:
  void SetWindowMode();
  void SetBaseLayout();
  void UpdateTextContent();
  void SetStyleSheet();
};

#endif
