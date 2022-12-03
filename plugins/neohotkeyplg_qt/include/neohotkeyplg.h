#ifndef NEOHOTKEYPLG_H
#define NEOHOTKEYPLG_H

#include <pluginobject.h>

class NeoHotKeyPlg: public PluginObject
{
protected:
  void InitMenuAction() override;
  void InitFunctionMap() override;
public:
  explicit NeoHotKeyPlg(YJson& settings);
  virtual ~NeoHotKeyPlg();
private:
  YJson& InitSettings(YJson& settings);
  void InitMenu();
};

#endif // NEOHOTKEYPLG_H