#ifndef NEOTRANSLATEPLG_H
#define NEOTRANSLATEPLG_H

#include <pluginobject.h>
#include <translatecfg.h>

class NeoTranslatePlg: public PluginObject
{
protected:
  class QAction* InitMenuAction() override;
public:
  explicit NeoTranslatePlg(YJson& settings);
  virtual ~NeoTranslatePlg();
private:
  YJson& InitSettings(YJson& settings);
  void InitFunctionMap() override;
private:
  TranslateCfg m_Settings;
  class QAction* m_MainMenuAction;
  class NeoTranslateDlg* m_TranslateDlg;
};

#endif // NEOTRANSLATEPLG_H

