#ifndef NEOSYSTEMTRAY_H
#define NEOSYSTEMTRAY_H

#include <QSystemTrayIcon>

class NeoSystemTray: public QSystemTrayIcon
{
public:
  explicit NeoSystemTray();
  virtual ~NeoSystemTray();
private:
  void InitDirs();
  void InitConnect();
};

#endif // NEOSYSTEMTRAY_H