#ifndef NATIVEEX_H
#define NATIVEEX_H

#include <functional>
#include <menubase.hpp>

class NativeExMenu: public MenuBase
{
public:
  explicit NativeExMenu(class YJson& data, MenuBase* parent, std::function<void(bool)> callback);
  virtual ~NativeExMenu();
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

#endif // NATIVEEX_H