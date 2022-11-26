#ifndef NEOSYSTEMPLG_H
#define NEOSYSTEMPLG_H

#include <pluginobject.h>

class NeoSystemPlg: public PluginObject
{
protected:
  void InitMenuAction(QMenu* pluginMenu) override;
public:
  explicit NeoSystemPlg(YJson& settings);
  virtual ~NeoSystemPlg();
private:
  void InitFunctionMap() override;
  static void LoadResources();
  static void SetDesktopRightMenu(bool on);
  static bool HasDesktopRightMenu();
  YJson& InitSettings(YJson& settings);
};

#endif // NEOSYSTEMPLG_H
