#ifndef WEATHERPLG_H
#define WEATHERPLG_H

#include <pluginobject.h>
#include <yjson.h>
#include <weathercfg.h>

class WeatherPlg: public PluginObject
{
public:
  explicit WeatherPlg(YJson& settings);
  virtual ~WeatherPlg();
private:
  class QAction* InitMenuAction() override;
  void InitFunctionMap() override;
  YJson& InitSettings(YJson& settings);
private:
  WeatherCfg m_Config;
  class QAction* m_MainMenuAction;
  class WeatherDlg* m_WeatherDlg;
};

#endif // WEATHERPLG_H