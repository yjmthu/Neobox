#ifndef WEATHER_HPP
#define WEATHER_HPP

#include <QObject>
#include <memory>

#include <yjson.h>

class Weather: public QObject
{
  Q_OBJECT

public:
  explicit Weather();
  virtual ~Weather();
  void FetchDays();
  void FetchCities();
  void FetchHours();
  std::optional<YJson> GetDays();
  std::optional<YJson> GetCities();
  std::optional<YJson> GetHours();
private:
  std::unique_ptr<class HttpLib> m_Request;
  std::mutex m_Mutex;
  std::optional<YJson> m_WeatherData;
  std::optional<YJson> m_CityList;
signals:
  void Finished(bool);
};

#endif // WEATHER_H