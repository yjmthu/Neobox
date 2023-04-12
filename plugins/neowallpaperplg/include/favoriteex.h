#ifndef FAVORITEEX_H
#define FAVORITEEX_H

#include <functional>
#include <wallbaseex.h>

class FavoriteExMenu: public WallBaseEx
{
public:
  explicit FavoriteExMenu(YJson data, MenuBase* parent, Callback callback);
  virtual ~FavoriteExMenu();
private:
  void LoadSettingMenu();
};

#endif // FAVORITEEX_H