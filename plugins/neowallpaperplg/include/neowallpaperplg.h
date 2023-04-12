#ifndef NEOWALLPAPERPLG_H
#define NEOWALLPAPERPLG_H

#include <pluginobject.h>

class NeoWallpaperPlg: public PluginObject
{
protected:
  class QAction* InitMenuAction() override;
public:
  explicit NeoWallpaperPlg(YJson& settings);
  virtual ~NeoWallpaperPlg();
private:
  YJson& InitSettings(YJson& settings);
  void InitFunctionMap() override;
  void LoadWallpaperTypeMenu(class MenuBase* pluginMenu);
  void LoadWallpaperExMenu(class MenuBase* pluginMenu);
  class Wallpaper* const m_Wallpaper;
  class MenuBase* m_WallpaperTypesMenu;
  class QAction* m_MoreSettingsAction;
  class QActionGroup* m_WallpaperTypesGroup;
  class QAction* m_MainMenuAction;
private:
  MenuBase* LoadWallavenMenu(MenuBase* parent);
  MenuBase* LoadBingApiMenu(MenuBase* parent);
  MenuBase* LoadDirectApiMenu(MenuBase* parent);
  MenuBase* LoadNativeMenu(MenuBase* parent);
  MenuBase* LoadScriptMenu(MenuBase* parent);
  MenuBase* LoadFavoriteMenu(MenuBase* parent);
  void LoadDropMenu(QAction* action);
  void LoadMainMenuAction();
};

#endif // NEOWALLPAPERPLG_H
