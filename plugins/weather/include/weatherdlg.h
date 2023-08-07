#ifndef WEATHERDLG_H
#define WEATHERDLG_H

#include <widgetbase.hpp>

class WeatherDlg: public WidgetBase {
protected:
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;
public:
  explicit WeatherDlg(class WeatherCfg& setting);
  virtual ~WeatherDlg();
  static void LoadQweatherFonts();
  static QString m_WeatherFontName;
  static std::map<std::u8string, int> m_FontsMap;
private:
  void LoadStyleSheet();
  void SetupUi();
  void InitComponent();
  void ConnectAll();
  void SearchCities(QString city);
  void ShowCityList(bool succeed);
  void UpdateItem(bool succeed);
  void UpdateHours(bool succeed);
  void UpdateDays(bool succeed);
  QString GetCityName() const;
  static void LoadFontsMap();
private:
  QPoint m_LastPosition;
  QWidget* const m_CenterWidget;
  class WeatherCfg& m_Config;
  class Weather* const m_Weather;
  class WeatherItem* const m_Item;
  class WeatherH* const m_Hours;
  class WeatherD* const m_Days;
  class QPushButton* const m_CityName;
  class CitySearch* const m_CityEdit;
  class CityList* const m_SearchList;
  class QPushButton* const m_Update;
};

#endif // WEATHERDLG_H
