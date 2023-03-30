#ifndef BINGAPIEX_H
#define BINGAPIEX_H

#include <functional>
#include <wallbaseex.h>

class BingApiExMenu: public WallBaseEx
{
public:
  explicit BingApiExMenu(YJson data, MenuBase* parent, Callback callback);
  virtual ~BingApiExMenu();
private:
  void LoadSettingMenu();
};

#endif // BINGAPIEX_H