#ifndef CITYLIST_HPP
#define CITYLIST_HPP

#include <QListWidget>
#include <yjson.h>

class WeatherCfg;

struct CityInfo {
  std::u8string name;
  std::u8string adm1;
  std::u8string adm2;
  std::u8string id;
  std::u8string GetCityName() {
    const std::u8string sep = u8" · ";
    auto result = adm1 + sep + adm2;
    if (name != adm2) {
      result += sep + name;
    }

    return result;
  }
  static CityInfo FromJSON(std::u8string_view id, const YJson& json) {
    auto& array = json.getArray();
    auto iter = array.begin();
    auto& adm1 = iter->getValueString();
    auto& adm2 = (++iter)->getValueString();
    auto& name = (++iter)->getValueString();

    return {
      .name = name,
      .adm1 = adm1,
      .adm2 = adm2,
      .id = std::u8string(id),
    };
  }

  YJson DumpJSON() const {
    return YJson::A { adm1, adm2, name };
  }
};

class CityList: public QListWidget
{
  Q_OBJECT

public:
  explicit CityList(QWidget* parent, WeatherCfg& cfg, class QPushButton& label, class QLineEdit& edit);
  void SetConetnt(const class YJson& data);
  void Move();
  void Show();
private:
  void ConnectAll();
private:
  QPushButton& m_Label;
  QLineEdit& m_Edit;
  std::vector<CityInfo> m_Info;
  WeatherCfg& m_Config;
signals:
  void CityChanged();
};

#endif // CITYLIST_HPP
