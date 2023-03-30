#ifndef NATIVEEX_H
#define NATIVEEX_H

#include <functional>
#include <wallbaseex.h>

class NativeExMenu: public WallBaseEx
{
public:
  explicit NativeExMenu(YJson data, MenuBase* parent, Callback callback);
  virtual ~NativeExMenu();
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

#endif // NATIVEEX_H