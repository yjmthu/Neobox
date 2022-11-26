#ifndef NEOSPEEDBOXPLG_H
#define NEOSPEEDBOXPLG_H

#include <pluginobject.h>

class NeoSpeedboxPlg: public PluginObject
{
protected:
  void InitMenuAction(QMenu *pluginMenu) override;
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
  void AddSkinConnect(class QAction* acion);
  void RemoveSkinConnect(QAction* action);
  void ChooseSkinConnect(QAction* action);
  class SpeedBox* m_Speedbox;
  class QMenu* m_ChooseSkinMenu;
  class QMenu* m_RemoveSkinMenu;
  class QActionGroup* m_ChooseSkinGroup;
};

#endif // NEOSPEEDBOXPLG_H
