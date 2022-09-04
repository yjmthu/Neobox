#ifndef NEOMENU_H
#define NEOMENU_H

#include <QMenu>

#include <functional>

class NeoMenu : public QMenu {
 protected:
  using FuncChecked = std::function<void(bool)>;
  using FuncCheck = std::function<bool()>;
  using FuncItemChecked = std::function<void(int)>;
  using FuncItemCheck = std::function<int()>;

 private:
  std::map<std::u8string, std::function<void(void)>> m_FuncNormalMap;
  std::map<std::u8string, std::pair<FuncChecked, FuncCheck>> m_FuncCheckMap;
  std::map<std::u8string, std::pair<FuncItemChecked, FuncItemCheck>>
      m_FuncItemCheckMap;
  std::map<std::u8string, QMenu *> m_ExMenus;

  void InitFunctionMap();
  void GetMenuContent(QMenu* parent, class YJson const& data);
  bool ChooseFolder(QString title, QString& folder);
  void LoadWallpaperExmenu();

 public:
  explicit NeoMenu(QWidget* parent = nullptr);
  ~NeoMenu();
  class Wallpaper*const m_Wallpaper;
};

#endif
