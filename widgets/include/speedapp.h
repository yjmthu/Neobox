#ifndef SPEEDAPP_H
#define SPEEDAPP_H

#include <QApplication>
#include <QObject>

class VarBox : public QObject {
  Q_OBJECT
 public:
  VarBox();
  ~VarBox();
  class Wallpaper* m_Wallpaper;
  class QQuickView* m_SpeedBox;
  class QSystemTrayIcon* m_Tray;
  class SpeedMenu* m_Menu;
  class Translater* m_Translater;

 private:
  class QTimer* m_Timer;
  friend class SpeedMenu;
  void GetSetting();
  void LoadFonts();
  void GetSpeedBox();
};

extern VarBox* m_VarBox;

#endif  // SPEEDAPP_H
