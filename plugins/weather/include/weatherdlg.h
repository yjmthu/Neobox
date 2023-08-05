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
  void ConnectAll();
private:
  QWidget* const m_CenterWidget;
  class YJson& m_Settings;
  class Weather* const m_Weather;
  class QPlainTextEdit* m_Text;
};

#endif // WEATHERDLG_H