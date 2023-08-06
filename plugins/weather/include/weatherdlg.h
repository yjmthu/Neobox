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
private:
  void LoadStyleSheet();
  void SetupUi();
  void InitComponent();
  void ConnectAll();
  void SearchCities(QString city);
  void ShowCityList(bool succeed);
  void UpdateItem(bool succeed);
  QString GetCityName() const;
private:
  QPoint m_LastPosition;
  QWidget* const m_CenterWidget;
  class WeatherCfg& m_Config;
  class Weather* const m_Weather;
  class WeatherItem* const m_Item;
  class QLabel* const m_CityName;
  class CitySearch* const m_CityEdit;
  class CityList* const m_SearchList;
  class QPlainTextEdit* m_Text;
};

#endif // WEATHERDLG_H
