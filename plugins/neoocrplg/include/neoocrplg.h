#ifndef NEOOCRPLG_H
#define NEOOCRPLG_H

#include <pluginobject.h>

class NeoOcrPlg: public PluginObject
{
protected:
  class QAction* InitMenuAction() override;
  void InitFunctionMap() override;
public:
  explicit NeoOcrPlg(YJson& settings);
  virtual ~NeoOcrPlg();
private:
  YJson& InitSettings(YJson& settings);
  void ChooseLanguages();
  void InitMenu();
private:
  QAction* m_MainMenuAction;
  class NeoOcr* const m_Ocr;
};

#endif // NEOOCRPLG_H
