#ifndef BINGAPIEX_H
#define BINGAPIEX_H

#include <functional>
#include <menubase.hpp>

class BingApiExMenu: public MenuBase
{
public:
  explicit BingApiExMenu(class YJson& data, MenuBase* parent, std::function<void(bool)> callback);
  virtual ~BingApiExMenu();
private:
  void LoadSettingMenu();
  const std::function<void(bool)> m_CallBack;
  YJson& m_Data;
};

#endif // BINGAPIEX_H