#ifndef NEOWALLPAPERPLG_H
#define NEOWALLPAPERPLG_H

#include <pluginobject.h>

class NeoWallpaperPlg: public PluginObject
{
protected:
  void InitMenuAction(QMenu *pluginMenu) override;
public:
  explicit NeoWallpaperPlg(YJson& settings);
  virtual ~NeoWallpaperPlg();
private:
  YJson& InitSettings(YJson& settings);
  void InitFunctionMap() override;
  void LoadWallpaperTypeMenu(class QMenu* pluginMenu);
  QMenu* LoadWallpaperExMenu(class QMenu* pluginMenu);
  class Wallpaper* const m_Wallpaper;
  class QMenu* m_WallpaperTypesMenu;
  class QAction* m_MoreSettingsAction;
  class QActionGroup* m_WallpaperTypesGroup;
private:
  QMenu* LoadWallavenMenu(QMenu* parent);
  QMenu* LoadBingApiMenu(QMenu* parent);
  QMenu* LoadDirectApiMenu(QMenu* parent);
  QMenu* LoadNativeMenu(QMenu* parent);
  QMenu* LoadScriptMenu(QMenu* parent);
  QMenu* LoadFavoriteMenu(QMenu* parent);
};

#endif // NEOWALLPAPERPLG_H
