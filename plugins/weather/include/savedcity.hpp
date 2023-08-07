#ifndef SAVEDCITY_HPP
#define SAVEDCITY_HPP

#include <widgetbase.hpp>
#include <weathercfg.h>

class SavedCityItem;

class SavedCity: public WidgetBase
{
  Q_OBJECT

public:
  explicit SavedCity(QWidget* parent, WeatherCfg& cfg);
  virtual ~SavedCity();
  void Update();
private:
  void SetupUi();
  void SetStyleSheet();
  void SetupCities();
  void ChangeCurrent(SavedCityItem* item);
  void DeleteItem(SavedCityItem* item);
  void SaveData();
private:
  WeatherCfg& m_Config;
  QWidget* const m_CenterWidget;
  class QListWidget*const m_List;
  SavedCityItem * m_LastItem = nullptr;
  std::u8string m_CurId;
  bool m_DataChanged = false;
signals:
  void Finished(bool changed);
};

#endif // SAVEDCITY_HPP