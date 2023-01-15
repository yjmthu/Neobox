#ifndef NEOCOLORPLG_H
#define NEOCOLORPLG_H

#include <pluginobject.h>

class NeoColorPlg: public PluginObject
{
protected:
  class QAction* InitMenuAction() override;
  void InitFunctionMap() override;
public:
  explicit NeoColorPlg(YJson& settings);
  virtual ~NeoColorPlg();
private:
  static YJson& InitSettings(YJson& settings);
private:
  QAction* m_MainMenuAction;
};

#endif // NEOCOLORPLG_H
