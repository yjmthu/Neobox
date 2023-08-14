#ifndef NEOMENU_HPP
#define NEOMENU_HPP

#include <neobox/menubase.hpp>

#include <functional>

class NeoMenu : public MenuBase {
  Q_OBJECT

public:
  explicit NeoMenu();
  virtual ~NeoMenu();
private:
  void InitStyleSheet();
  void InitPluginMenu();
  void InitFunctionMap();
  void InitSettingMenu();
  bool IsAutoStart();
  void SetAutoSatrt(QAction* action, bool on);
public:
  MenuBase* const m_SettingMenu;
  QAction* const m_ControlPanel;
  MenuBase* const m_PluginMenu;
};

#endif
