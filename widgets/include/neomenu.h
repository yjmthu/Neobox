#ifndef NEOMENU_H
#define NEOMENU_H

#include <QMenu>

#include <functional>

class NeoMenu : public QMenu {
 protected:
  using FuncNormal = std::function<void(void)>;
  using FuncChecked = std::function<void(bool)>;
  using FuncCheck = std::function<bool()>;
  using FuncItemChecked = std::function<void(int)>;
  using FuncItemCheck = std::function<int()>;

 private:
  std::map<std::u8string, FuncNormal> m_FuncNormalMap;
  std::map<std::u8string, std::pair<FuncChecked, FuncCheck>> m_FuncCheckMap;
  std::map<std::u8string, std::pair<FuncItemChecked, FuncItemCheck>>
      m_FuncItemCheckMap;

  void InitWallpaper();
  void InitFunctionMap();
  void GetMenuContent(QMenu* parent, class YJson const& data);
  bool ChooseFolder(QString title, QString& folder);

 public:
  explicit NeoMenu(QWidget* parent = nullptr);
  ~NeoMenu();
  class Wallpaper*const m_Wallpaper;
};

#endif
