#ifndef NEOSPEEDBOXPLG_H
#define NEOSPEEDBOXPLG_H

#include <pluginobject.h>
#include <speedboxcfg.h>

class MenuBase;

namespace std::filesystem {
  class path;
}

class NeoSpeedboxPlg: public PluginObject
{
protected:
  class QAction* InitMenuAction() override;
public:
  explicit NeoSpeedboxPlg(YJson& settings);
  virtual ~NeoSpeedboxPlg();
private:
  SpeedBoxCfg m_Settings;
public:
  class NetSpeedHelper* m_NetSpeedHelper;
  static int IsDefaultSkin(const std::u8string& key);
private:
  YJson& InitSettings(YJson& settings);
  void InitFunctionMap() override;
  void InitMenu();
  // static void LoadFonts();
  void LoadRemoveSkinMenu(MenuBase* parent);
  void LoadChooseSkinMenu(MenuBase* parent);
  void LoadHideAsideMenu(MenuBase* parent);
  void AddSkinConnect(class QAction* acion);
  void RemoveSkinConnect(QAction* action);
  void ChooseSkinConnect(QAction* action);
  void AddSkin(const QString& name, const std::filesystem::path& path);
private:
  static const std::vector<std::u8string> m_DefaultSkins;
  class SpeedBox* m_Speedbox;
  MenuBase* m_ChooseSkinMenu;
  MenuBase* m_RemoveSkinMenu;
  MenuBase* m_NetCardMenu;
  class QActionGroup* m_ChooseSkinGroup;
  std::function<void(PluginEvent, void*)> m_ActiveWinodow;
};

#endif // NEOSPEEDBOXPLG_H
