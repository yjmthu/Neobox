#ifndef WEATHERITEM_HPP
#define WEATHERITEM_HPP

#include <QWidget>
#include <yjson.h>

class WeatherItem: public QWidget
{
  Q_OBJECT

public:
  explicit WeatherItem(QWidget* parent);
  virtual ~WeatherItem();
private:
  void SetupUi();
  static std::map<std::u8string, int> m_FontsMap;
  static void LoadFontsMap();
};

#endif // WEATHERITEM_HPP