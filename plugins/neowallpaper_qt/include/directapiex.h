#ifndef DIRECTAPI_H
#define DIRECTAPI_H

#include <functional>
#include <wallbaseex.h>

class DirectApiExMenu: public WallBaseEx
{
public:
  explicit DirectApiExMenu(YJson data, MenuBase* parent, Callback callback);
  virtual ~DirectApiExMenu();
private:
  void LoadSettingMenu();
  void LoadSubSettingMenu(QAction* action);
  void LoadPaths(MenuBase* subMenu, const std::u8string& name);
  void AddApi();
  void EditApi(QAction* action);
  void RenameApi(QAction* action);
  class QAction* const m_Separator;
  class QActionGroup* m_ActionGroup;
};


#endif // DIRECTAPI_H