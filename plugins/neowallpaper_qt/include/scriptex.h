#ifndef SCRIPTEX_H
#define SCRIPTEX_H

#include <functional>
#include <QMenu>

class ScriptExMenu: public QMenu
{
public:
  explicit ScriptExMenu(class YJson& data, QMenu* parent, std::function<void(bool)> callback);
  virtual ~ScriptExMenu();
private:
  void LoadSettingMenu();
  void LoadTypeChooseMenu();
  void LoadDataEditMenu();
  const std::function<void(bool)> m_CallBack;
  YJson& m_Data;
};

#endif // SCRIPTEX_H