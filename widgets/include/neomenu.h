#ifndef NEOMENU_H
#define NEOMENU_H

#include <QMenu>

#include <functional>

class NeoMenu : public QMenu {
 public:
  explicit NeoMenu(QWidget* parent = nullptr);
  virtual ~NeoMenu();
  void InitPluginMgr();
 private:
  void InitStyleSheet();
  void InitPluginMenu();
  void InitFunctionMap();
  void InitSettingMenu();
  static bool IsAutoStart();
  static void SetAutoSatrt(QAction* action, bool on);
  QMenu *m_SettingMenu, *m_PluginMenu;
  class PluginMgr* m_PluginMgr;
};

#endif
