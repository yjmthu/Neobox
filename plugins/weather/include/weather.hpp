#ifndef WEATHER_HPP
#define WEATHER_HPP

#include <QObject>
#include <memory>

#include <yjson.h>
#include <weathercfg.h>
#include <httplib.h>

class Weather: public QObject
{
  Q_OBJECT

public:
  enum class GetTypes { Cities, Hours, Days, Now };
  explicit Weather(const WeatherCfg& config);
  virtual ~Weather();
  void Fetch(GetTypes type, std::optional<std::u8string_view> data=std::nullopt);
  std::optional<YJson> Get();
private:
  HttpUrl GetUrl(GetTypes type, std::optional<std::u8string_view> data) const;
private:
  std::unique_ptr<class HttpLib> m_Request;
  std::mutex m_Mutex;
  std::optional<YJson> m_JSON;
  const WeatherCfg& m_Config;
signals:
  void Finished(GetTypes, bool);
};

#endif // WEATHER_H