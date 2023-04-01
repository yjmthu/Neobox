#ifndef WEATHER_H
#define WEATHER_H

#include <widgetbase.hpp>

class WeatherDlg: public WidgetBase {
public:
  explicit WeatherDlg(class YJson& setting);
  virtual ~WeatherDlg();
private:
  class YJson& m_Settings;
};

#endif // WEATHER_H