#ifndef NEOOCRPLG_H
#define NEOOCRPLG_H

#include <pluginobject.h>
#include <ocrconfig.h>

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
#ifdef _WIN32
  void AddWindowsSection(QWidget* parent, QVBoxLayout* layout);
#endif
  void AddTesseractSection(QWidget* parent, QVBoxLayout* layout);
private:
  OcrConfig m_Settings;
  class OcrDialog* m_OcrDialog;
  QAction* m_MainMenuAction;
  class NeoOcr* const m_Ocr;
};

#endif // NEOOCRPLG_H
