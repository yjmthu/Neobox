#ifndef DIRECTAPI_H
#define DIRECTAPI_H

#include <functional>
#include <menubase.hpp>

class DirectApiExMenu: public MenuBase
{
public:
  explicit DirectApiExMenu(class YJson& data, MenuBase* parent, std::function<void(bool)> callback);
  virtual ~DirectApiExMenu();
private:
  void LoadSettingMenu();
  void LoadSubSettingMenu(QAction* action);
  void LoadPaths(MenuBase* subMenu, const std::u8string& name);
  void AddApi();
  void EditApi(QAction* action);
  void RenameApi(QAction* action);
  const std::function<void(bool)> m_CallBack;
  YJson& m_Data;
  class QAction* const m_Separator;
  class QActionGroup* m_ActionGroup;
};


#endif // DIRECTAPI_H