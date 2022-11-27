#ifndef NEOMENU_H
#define NEOMENU_H

#include <QMenu>

#include <functional>

class NeoMenu : public QMenu {
 public:
  explicit NeoMenu(class GlbObject* glb);
  virtual ~NeoMenu();
  void InitPluginMgr();
 private:
  void InitStyleSheet();
  void InitPluginMenu();
  void InitFunctionMap();
  void InitSettingMenu();
  bool IsAutoStart();
  void SetAutoSatrt(QAction* action, bool on);
  GlbObject* m_GlbObject;
  QMenu *m_SettingMenu, *m_PluginMenu;
  class PluginMgr* m_PluginMgr;
};

#endif
