#ifndef SCRIPTEX_H
#define SCRIPTEX_H

#include <functional>
#include <menubase.hpp>

class ScriptExMenu: public MenuBase
{
public:
  explicit ScriptExMenu(class YJson& data, MenuBase* parent, std::function<void(bool)> callback);
  virtual ~ScriptExMenu();
private:
  void LoadSubSettingMenu(QAction* action);
  void LoadSettingMenu();
  void AddApi();
  void EditApi(QAction* action);
  void RenameApi(QAction* action);
private:
  const std::function<void(bool)> m_CallBack;
  class QAction* const m_Separator;
  class QActionGroup* m_ActionGroup;
  YJson& m_Data;
};

#endif // SCRIPTEX_H