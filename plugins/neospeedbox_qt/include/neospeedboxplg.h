#ifndef NEOSPEEDBOXPLG_H
#define NEOSPEEDBOXPLG_H

#include <pluginobject.h>

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
  YJson& InitSettings(YJson& settings);
  void InitFunctionMap() override;
  void InitMenu();
  static void LoadFonts();
  void LoadRemoveSkinMenu(QMenu* parent);
  void LoadChooseSkinMenu(QMenu* parent);
  void LoadHideAsideMenu(QMenu* parent);
  void AddSkinConnect(class QAction* acion);
  void RemoveSkinConnect(QAction* action);
  void ChooseSkinConnect(QAction* action);
  void AddSkin(const QString& name, const std::filesystem::path& path);
private:
  class SpeedBox* m_Speedbox;
  class QMenu* m_ChooseSkinMenu;
  class QMenu* m_RemoveSkinMenu;
  class QMenu* m_NetCardMenu;
  class QActionGroup* m_ChooseSkinGroup;
  std::function<void(PluginEvent, void*)> m_ActiveWinodow;
};

#endif // NEOSPEEDBOXPLG_H
