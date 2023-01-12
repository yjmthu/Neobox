#ifndef NEOSYSTEMTRAY_HPP
#define NEOSYSTEMTRAY_HPP

#include <QSystemTrayIcon>
#include <pluginobject.h>

class NeoSystemTray: public QSystemTrayIcon
{
  Q_OBJECT

public:
  explicit NeoSystemTray();
  virtual ~NeoSystemTray();
  std::set<const PluginObject::FollowerFunction*> m_Followers;
private:
  void InitDirs();
  void InitConnect();
};

#endif // NEOSYSTEMTRAY_HPP