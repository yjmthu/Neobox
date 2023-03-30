#ifndef SCRIPTEX_H
#define SCRIPTEX_H

#include <functional>
#include <wallbaseex.h>

class ScriptExMenu: public WallBaseEx
{
public:
  explicit ScriptExMenu(YJson data, MenuBase* parent, Callback callback);
  virtual ~ScriptExMenu();
private:
  void LoadSubSettingMenu(QAction* action);
  void LoadSettingMenu();
  void AddApi();
  void EditApi(QAction* action);
  void RenameApi(QAction* action);
private:
  class QAction* const m_Separator;
  class QActionGroup* m_ActionGroup;
};

#endif // SCRIPTEX_H