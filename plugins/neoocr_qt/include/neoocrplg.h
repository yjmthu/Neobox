#ifndef NEOOCRPLG_H
#define NEOOCRPLG_H

#include <pluginobject.h>

class NeoOcrPlg: public PluginObject
{
protected:
  void InitMenuAction(QMenu* menu) override;
  void InitFunctionMap() override;
public:
  explicit NeoOcrPlg(YJson& settings);
  virtual ~NeoOcrPlg();
private:
  YJson& InitSettings(YJson& settings);
  void InitMenu();
  class NeoOcr* const m_Ocr;
};

#endif // NEOOCRPLG_H
