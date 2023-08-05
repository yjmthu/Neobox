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
  void SendRequest();
  std::optional<YJson> GetResult();
private:
  std::unique_ptr<class HttpLib> m_Request;
  std::mutex m_Mutex;
  std::optional<YJson> m_Result;
signals:
  void Finished(bool);
};

#endif // WEATHER_H