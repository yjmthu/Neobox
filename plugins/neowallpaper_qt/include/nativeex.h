#ifndef NATIVEEX_H
#define NATIVEEX_H

#include <functional>
#include <QMenu>

class NativeExMenu: public QMenu
{
public:
  explicit NativeExMenu(class YJson& data, QMenu* parent, std::function<void(bool)> callback);
  virtual ~NativeExMenu();
private:
  void LoadSettingMenu();
  const std::function<void(bool)> m_CallBack;
  YJson& m_Data;
};

#endif // NATIVEEX_H