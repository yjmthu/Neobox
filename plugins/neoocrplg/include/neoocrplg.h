#ifndef NEOOCRPLG_H
#define NEOOCRPLG_H

#include <pluginobject.h>

class QWidget;
class QVBoxLayout;

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
  void AddEngineMenu();
  void AddWindowsSection(QWidget* parent, QVBoxLayout* layout);
  void AddTesseractSection(QWidget* parent, QVBoxLayout* layout);
private:
  QAction* m_MainMenuAction;
  class NeoOcr* const m_Ocr;
};

#endif // NEOOCRPLG_H
