#ifndef WEATHERITEM_HPP
#define WEATHERITEM_HPP

#include <QFrame>
#include <yjson.h>
#include <QLabel>


struct WeatherDetail {
  std::u8string icon;
  QString name;
  std::u8string count;
  QString unit;
  QLabel*const label = new QLabel;
  void SetLabelText() {
    auto text = QString::fromUtf8(count.data(), count.size());
    text += QStringLiteral("<small style=\"font-size:10px;\">%1</small>").arg(unit);
    label->setText(text);
  }
};

class WeatherItem: public QFrame
{
  Q_OBJECT

public:
  explicit WeatherItem(QWidget* parent);
  virtual ~WeatherItem();
  void SetJSON(const YJson& json);
private:
  void SetupUi();
  class QGridLayout* SetupGrid();
  void SetObjectName();
  void SetTemp(std::u8string_view value);
  void SetDayText(std::u8string_view value);
  void SetDayIcon(const std::u8string& value);
private:
private:
  QLabel*const m_IconDayNight;
  QLabel*const m_TextDayNight;
  QLabel*const m_Temperature;
  std::map<std::u8string, WeatherDetail> m_DetailMap;
};

#endif // WEATHERITEM_HPP