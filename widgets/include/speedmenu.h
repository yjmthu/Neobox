#ifndef SPEEDMENU_H
#define SPEEDMENU_H

#include <QObject>

class SpeedMenu : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool appAutoStart READ appAutoStart)

 public:
  explicit SpeedMenu();
  ~SpeedMenu();
  Q_INVOKABLE bool toolOcrEnableScreenShotCut(const QString& keys, bool enable);
  Q_INVOKABLE static void appShutdownComputer();
  Q_INVOKABLE static void appRestartComputer();
  Q_INVOKABLE static void appOpenDir(const QString& path);
  Q_INVOKABLE void wallpaperGetNext();
  Q_INVOKABLE void wallpaperGetPrev();
  Q_INVOKABLE void wallpaperRemoveCurrent();

 private:
  class Wallpaper* m_Wallpaper;

 private:
  bool appAutoStart();

 public slots:
  Q_INVOKABLE void toolOcrGetScreenShotCut();

 private slots:
};

#endif  // SPEEDMENU_H
