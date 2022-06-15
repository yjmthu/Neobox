#ifndef SPEEDBOX_H
#define SPEEDBOX_H

#include <QObject>

class SpeedBox : public QObject {
  Q_OBJECT
//  void dragEnterEvent(QDragEnterEvent* event) override;
//  void dropEvent(QDropEvent* event) override;

 public:
  SpeedBox();
  ~SpeedBox();
  Q_INVOKABLE void updateInfo();
  Q_INVOKABLE void showMenu(int x, int y);
  Q_INVOKABLE void setRoundRect(int x, int y, int w, int h, int r, bool set);

 private:
  friend class SpeedMenu;
  class NetSpeedHelper* m_NetSpeedHelper;
};
#endif  // SPEEDBOX_H
