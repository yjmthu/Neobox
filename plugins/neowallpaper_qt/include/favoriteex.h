#ifndef FAVORITEEX_H
#define FAVORITEEX_H

#include <functional>
#include <menubase.hpp>

class FavoriteExMenu: public MenuBase
{
public:
  explicit FavoriteExMenu(class YJson& data, MenuBase* parent, std::function<void(bool)> callback);
  virtual ~FavoriteExMenu();
private:
  void LoadSettingMenu();
private:
  const std::function<void(bool)> m_CallBack;
  YJson& m_Data;
};

#endif // FAVORITEEX_H