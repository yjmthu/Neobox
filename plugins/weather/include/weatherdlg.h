#ifndef WEATHERDLG_H
#define WEATHERDLG_H

#include <widgetbase.hpp>

class WeatherDlg: public WidgetBase {
public:
  explicit WeatherDlg(class YJson& setting);
  virtual ~WeatherDlg();
private:
  void SetupUi();
  void InitComponent();
private:
  QWidget* m_CenterWidget;
  class YJson& m_Settings;
};

#endif // WEATHERDLG_H